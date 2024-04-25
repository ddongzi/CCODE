//
// Created by dong on 4/25/24.
//
#include <fcntl.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
/* 与getline交互*/
typedef struct {
    char *buffer;
    size_t buffer_length;
    size_t input_length;
} input_buffer_t;

/*源命令 结果*/
typedef enum {
    META_COMMAND_SUCCESS, // 源命令识别成功
    META_COMMAND_UNRECOGNIZED_COMMAND // 。开始， 但是命令不存在
} meta_cmd_res;

/*sql命令 结果*/
typedef enum {
    PREPARE_SUCCESS,
    PREPARE_UNRECOGNIZED_STATEMENT,
    PREPARE_SYNTAX_ERROR,
    PREPARE_STRING_TOO_LONG,
    PREPARE_NEGATIVE_ID
} prepare_res_type;

/*sql 命令类型*/
typedef enum {
    STATEMENT_INSERT,
    STATEMENT_SELECT
} statement_type;

#define COLUMN_USERNAME_SIZE 32
#define COLUMN_EMAIL_SIZE 255
typedef struct {
    uint32_t id;
    char user_name[COLUMN_USERNAME_SIZE + 1];
    char email[COLUMN_EMAIL_SIZE + 1];
} row_t;

typedef struct {
    statement_type type;
    row_t row_to_insert;
} sql_statement;



// 命令执行结果
typedef enum { EXECUTE_SUCCESS, EXECUTE_TABLE_FULL } execute_res;

#define size_of_attribute(type, attr) sizeof(((type*)0)->attr)

const uint32_t ID_SIZE = size_of_attribute(row_t, id);
const uint32_t USER_NAME_SIZE = size_of_attribute(row_t, user_name);
const uint32_t EMAIL_SIZE = size_of_attribute(row_t, email);
const uint32_t ID_OFFSET = 0;
const uint32_t USER_NAME_OFFSET = ID_OFFSET + ID_SIZE;
const uint32_t EMAIL_OFFSET = USER_NAME_OFFSET + USER_NAME_SIZE;
const uint32_t ROW_SIZE = ID_SIZE + USER_NAME_SIZE + EMAIL_SIZE;

void serialize_row(row_t* source, void* destination);

const uint32_t PAGE_SIZE = 4096; // 每个页 4096 个字节
#define TABLE_MAX_PAGES 100
const uint32_t ROWS_PER_PAGE = PAGE_SIZE / ROW_SIZE; // 每个页 容纳的行数 : 14
const uint32_t TABLE_MAX_ROWS = ROWS_PER_PAGE * TABLE_MAX_PAGES; // 表格容纳最大行数: 1400

/*使用 pager 访问页缓存和文件*/
typedef struct {
    int fd;
    uint32_t file_length; // 追加写。 写的起始位置. 是page_size 整数倍
    void *pages[TABLE_MAX_PAGES];
} pager_t;

typedef struct {
    uint32_t num_rows;
    pager_t* pager;
} table_t;



void close_input_buffer(input_buffer_t* input_buffer);

/*
 * 根据page_num获取page：0，1，2....
 * 如果没有命中缓存，从文件读取上来
 * 如果命中缓存，直接返回。
 * */
void *get_page(pager_t* pager, uint32_t page_num)
{
    if (page_num >= TABLE_MAX_PAGES) {
        printf("Tried to fetch page number out of bounds. %d > %d\n", page_num,
               TABLE_MAX_PAGES);
        exit(EXIT_FAILURE);
    }

    if (pager->pages[page_num] == NULL) {
        // 未命中缓存， 分配内存，从文件读
        void *page = malloc(PAGE_SIZE);
        uint32_t num_pages = pager->file_length / PAGE_SIZE; // 获取page个数

        // We might save a partial page at the end of the file.
        if (pager->file_length % PAGE_SIZE) {
            num_pages += 1;
        }

        if (page_num < num_pages) {
            lseek(pager->fd, page_num * PAGE_SIZE, SEEK_SET);
            ssize_t bytes_read = read(pager->fd, page, PAGE_SIZE);
            if (bytes_read == -1) {
                printf("Error reading file: %d\n", errno);
                exit(EXIT_FAILURE);
            }
        }
        pager->pages[page_num] = page;
    }
    return pager->pages[page_num];

}
/*将page刷盘*/
void pager_flush(pager_t *pager, uint32_t page_num, uint32_t size)
{
    if (pager->pages[page_num] == NULL) {
        printf("Tried to flush null page.\n");
        exit(EXIT_FAILURE);
    }
    off_t offset = lseek(pager->fd, page_num * PAGE_SIZE, SEEK_SET);
    if (offset == -1) {
        printf("Error seeking: %d\n", errno);
        exit(EXIT_FAILURE);
    }
    ssize_t bytes_written = write(pager->fd, pager->pages[page_num], size);
    if (bytes_written == -1) {
        printf("Error writing: %d\n", errno);
        exit(EXIT_FAILURE);
    }
}
/*
 * 1. 缓存刷新到盘
 * 2. 关闭db file
 * 3. 释放table， pager
 * */
void db_close(table_t *table)
{
    pager_t *pager = table->pager;
    uint32_t num_full_pages = table->num_rows / ROWS_PER_PAGE;
    for (uint32_t i = 0; i < num_full_pages; ++i) {
        if (pager->pages[i] == NULL) {
            continue;
        }
        pager_flush(pager, i, PAGE_SIZE);
        free(pager->pages[i]);
        pager->pages[i] = NULL;
    }
    uint32_t num_additional_rows = table->num_rows % ROWS_PER_PAGE;
    if (num_additional_rows > 0) {
        uint32_t page_num = num_full_pages;
        pager_flush(pager, page_num, num_additional_rows * ROW_SIZE);
        free(pager->pages[page_num]);
        pager->pages[page_num] = NULL;
    }

    int res = close(pager->fd);
    if (res == -1) {
        printf("Error closing db file.\n");
        exit(EXIT_FAILURE);
    }

    for (uint32_t i = 0; i < TABLE_MAX_PAGES; ++i) {
        void *page = pager->pages[i];
        if (page) {
            free(page);
            pager->pages[i] = NULL;
        }
    }
    free(pager);
    free(table);
}

// 定位 : 返回第row_num在table中位置
void * row_slot(table_t *table, uint32_t row_num)
{
    uint32_t page_num = row_num/ROWS_PER_PAGE;
    void* page = get_page(table->pager, page_num);
    uint32_t row_offset = row_num % ROWS_PER_PAGE;
    uint32_t byte_offset = row_offset * ROW_SIZE;
    return page + byte_offset;
}

/* row_t 落入内存*/
void serialize_row(row_t* source, void* destination)
{
    memcpy(destination + ID_OFFSET, &(source->id), ID_SIZE);
    strncpy(destination + USER_NAME_OFFSET, source->user_name, USER_NAME_SIZE);
    strncpy(destination + EMAIL_OFFSET, source->email, EMAIL_SIZE);
}

void print_row(row_t *row)
{
    printf("(%d, %s, %s)\n", row->id, row->user_name, row->email);
}

/* 从内存读,  source 为 一行的起始地址*/
void deserialize_row(void* source, row_t* destination) {
    memcpy(&(destination->id), source + ID_OFFSET, ID_SIZE);
    memcpy(&(destination->user_name), source + USER_NAME_OFFSET, USER_NAME_SIZE);
    memcpy(&(destination->email), source + EMAIL_OFFSET, EMAIL_SIZE);
}

/* 执行select， 从table.page内存读出*/
execute_res execute_select(sql_statement * statement, table_t * table)
{
    row_t row;
    for (uint32_t i = 0; i < table->num_rows; ++i) {
        deserialize_row(row_slot(table, i), &row);
        print_row(&row);
    }
    return EXECUTE_SUCCESS;
}

/* 执行insert， 写入table.page内存*/
execute_res execute_insert(sql_statement* statement, table_t* table)
{
    if (table->num_rows  >= TABLE_MAX_ROWS) {
        return EXECUTE_TABLE_FULL;
    }
    row_t* row = &(statement->row_to_insert);
    serialize_row(row, row_slot(table, table->num_rows));
    table->num_rows += 1;
    return EXECUTE_SUCCESS;
}
/*执行SQL 语句*/
execute_res execute_statement(sql_statement *statement, table_t *table)
{
    switch (statement->type) {
        case STATEMENT_INSERT:
            return execute_insert(statement, table);
        case STATEMENT_SELECT:
            return execute_select(statement, table);
    }
}

/*
 * 打开文件， 初始化pager
 * */
pager_t *pager_open(const char *file_name)
{
    // 文件读写， 文件所有者读写权限
    int fd = open(file_name, O_RDWR|O_CREAT, S_IWUSR|S_IRUSR);
    if (fd < 0) {
        printf("Unable to open file.\n");
        exit(EXIT_FAILURE);
    }

    off_t file_length = lseek(fd, 0, SEEK_END);

    pager_t *pager = (pager_t *) malloc(sizeof(pager_t));
    pager->fd = fd;
    pager->file_length = file_length;

    for (uint32_t i = 0; i < TABLE_MAX_PAGES; ++i) {
        pager->pages[i] = NULL;
    }
    
    return pager;
}

/* db初始化
 * 1. 打开db file
 * 2. 初始化pager
 * 3. 初始化table
 * */
table_t *db_open(const char *file_name)
{
    pager_t *pager = pager_open(file_name);
    uint32_t num_rows = pager->file_length / ROW_SIZE;

    table_t *table = (table_t *) malloc(sizeof(table_t));
    table->pager = pager;
    table->num_rows = num_rows;
    return table;
}

/* 处理insert*/
prepare_res_type prepare_insert(input_buffer_t* input_buffer, sql_statement *statement)
{
    statement->type = STATEMENT_INSERT;
    char *keyword = strtok(input_buffer->buffer, " ");
    char *id_str = strtok(NULL, " ");
    char *username = strtok(NULL, " ");
    char *email = strtok(NULL, " ");

    if (id_str == NULL || username == NULL || email == NULL) {
        return PREPARE_SYNTAX_ERROR;
    }
    int id = atoi(id_str);
    if (id < 0) {
        return PREPARE_NEGATIVE_ID;
    }
    if (strlen(username) > COLUMN_USERNAME_SIZE) {
        return PREPARE_STRING_TOO_LONG;
    }
    if (strlen(email) > COLUMN_EMAIL_SIZE) {
        return PREPARE_STRING_TOO_LONG;
    }
    statement->row_to_insert.id = id;
    strcpy(statement->row_to_insert.user_name, username);
    strcpy(statement->row_to_insert.email, email);
    return PREPARE_SUCCESS;
}

/* 填充 statement*/
/*
 * insert 1 cstack foo@bar.com
*/
prepare_res_type prepare_statement(input_buffer_t *input_buffer, sql_statement *statement)
{
    if (strncmp(input_buffer->buffer, "insert", 6) == 0) {
        return prepare_insert(input_buffer, statement);
    }
    if (strncmp(input_buffer->buffer, "select", 6) == 0) {
        statement->type = STATEMENT_SELECT;
        return PREPARE_SUCCESS;
    }
    return PREPARE_UNRECOGNIZED_STATEMENT;
}

/*处理源命令*/
meta_cmd_res do_meta_command(input_buffer_t *input_buffer, table_t *table)
{
    if (strcmp(input_buffer->buffer, ".exit") == 0) {
        close_input_buffer(input_buffer);
        db_close(table);
        exit(EXIT_SUCCESS);
    } else {
        return META_COMMAND_UNRECOGNIZED_COMMAND;
    }
}

input_buffer_t* new_input_buffer()
{
    input_buffer_t* input_buffer = (input_buffer_t*)malloc(sizeof(input_buffer_t));
    input_buffer->buffer = NULL;
    input_buffer->buffer_length = 0;
    input_buffer->input_length = 0;

    return input_buffer;
}
void close_input_buffer(input_buffer_t* input_buffer)
{
    free(input_buffer->buffer);
    free(input_buffer);
}
void read_input(input_buffer_t *input_buffer)
{
    ssize_t bytes_read = getline(&(input_buffer->buffer), &(input_buffer->buffer_length), stdin);
    if (bytes_read < 0) {
        printf("Error reading input");
        exit(EXIT_FAILURE);
    }
    // 去除换行符号
    input_buffer->input_length = bytes_read - 1;
    input_buffer->buffer[bytes_read - 1] = '\0';
}
// 打印提示
void print_prompt()
{
    printf("db > ");
}

/**
 *

 ~ sqlite3
SQLite version 3.16.0 2016-11-04 19:09:39
Enter ".help" for usage hints.
Connected to a transient in-memory database.
Use ".open FILENAME" to reopen on a persistent database.
sqlite> create table users (id int, username varchar(255), email varchar(255));
sqlite> .tables
users
sqlite> .exit
 * @param argc
 * @param argv
 * @return
 */

int main(int argc, char *argv[])
{
    if (argc < 2) {
        printf("Must supply a  db filename.\n");
        exit(EXIT_FAILURE);
    }
    char *file_name = argv[1];
    table_t *table = db_open(file_name);
    input_buffer_t *input_buffer = new_input_buffer();
    //printf("PAGES: %u, ROWS-PAGE: %u, ROW-SIZE :%u\n", TABLE_MAX_PAGES, ROWS_PER_PAGE, ROW_SIZE);
    while (true) {
        /*1. Making a Simple REPL*/
        print_prompt();
        read_input(input_buffer);
        if (input_buffer->buffer[0] == '.') {
            // 源命令
            switch (do_meta_command(input_buffer, table)) {
                case META_COMMAND_SUCCESS:
                    continue;
                case META_COMMAND_UNRECOGNIZED_COMMAND:
                    printf("Unrecognized command `%s` \n", input_buffer->buffer);
                    continue;
            }
        }
        sql_statement statement;
        switch (prepare_statement(input_buffer, &statement)) {
            // 拿到SQL 就去处理
            case PREPARE_SUCCESS:
                break;
            case PREPARE_SYNTAX_ERROR:
                printf("Syntax error. Could not parse statement.\n");
                continue;
            case PREPARE_UNRECOGNIZED_STATEMENT:
                printf("Unrecognized SQL command `%s` \n", input_buffer->buffer);
                continue;
            case PREPARE_STRING_TOO_LONG:
                printf("String is too long.\n");
                continue;
            case PREPARE_NEGATIVE_ID:
                printf("ID must be positive.\n");
                continue;
        }

        switch (execute_statement(&statement, table)) {
            case EXECUTE_SUCCESS:
                printf("Executed.\n");
                break;
            case EXECUTE_TABLE_FULL:
                printf("Error: Table full.\n");
                break;
        }
    }
    return 0;
}