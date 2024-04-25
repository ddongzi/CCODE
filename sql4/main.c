//
// Created by dong on 4/25/24.
//

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>

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
    PREPARE_SYNTAX_ERROR
} sql_cmd_res;

/*sql 命令类型*/
typedef enum {
    STATEMENT_INSERT,
    STATEMENT_SELECT
} statement_type;

#define COLUMN_USERNAME_SIZE 32
#define COLUMN_EMAIL_SIZE 255
typedef struct {
    uint32_t id;
    char user_name[COLUMN_USERNAME_SIZE];
    char email[COLUMN_EMAIL_SIZE];
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
const uint32_t ROWS_PER_PAGE = PAGE_SIZE / ROW_SIZE; // 每个页 容纳的行数
const uint32_t TABLE_MAX_ROWS = ROWS_PER_PAGE * TABLE_MAX_PAGES; // 表格容纳最大行数

typedef struct {
    uint32_t num_rows;
    void *pages[TABLE_MAX_PAGES]; // 维持每一个页，
} table_t;

// 定位 : 返回第row_num在table中位置
void * row_slot(table_t *table_t, uint32_t row_num)
{
    uint32_t page_num = row_num/ROWS_PER_PAGE;
    void* page = table_t->pages[page_num];
    if (page == NULL) {
        // 创建一个页
        page = table_t->pages[page_num] = malloc(sizeof(PAGE_SIZE));
    }
    uint32_t row_offset = row_num % ROWS_PER_PAGE;
    uint32_t byte_offset = row_offset * ROW_SIZE;
    return page + byte_offset;
}



/* row_t 落入内存*/
void serialize_row(row_t* source, void* destination)
{
    memcpy(destination + ID_OFFSET, &(source->id), ID_SIZE);
    memcpy(destination + USER_NAME_OFFSET, &(source->user_name), USER_NAME_SIZE);
    memcpy(destination + EMAIL_OFFSET, &(source->email), EMAIL_SIZE);
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
execute_res execute_select(sql_statement * statement, table_t * table_t)
{
    row_t row;
    for (uint32_t i = 0; i < table_t->num_rows; ++i) {
        deserialize_row(row_slot(table_t, i), &row);
        print_row(&row);
    }
    return EXECUTE_SUCCESS;
}

/* 执行insert， 写入table.page内存*/
execute_res execute_insert(sql_statement* statement, table_t* table_t)
{
    if (table_t->num_rows >= TABLE_MAX_ROWS) {
        return EXECUTE_TABLE_FULL;
    }
    row_t* row = &(statement->row_to_insert);
    serialize_row(row, row_slot(table_t, table_t->num_rows));
    table_t->num_rows += 1;
    return EXECUTE_SUCCESS;
}
/*执行SQL 语句*/
execute_res execute_statement(sql_statement *statement, table_t *table_t)
{
    switch (statement->type) {
        case STATEMENT_INSERT:
            return execute_insert(statement, table_t);
        case STATEMENT_SELECT:
            return execute_select(statement, table_t);
    }
}
/* 表初始化*/
table_t *new_table()
{
    table_t *table = (table_t *) malloc(sizeof(table_t));
    table->num_rows = 0;
    for (int i = 0; i < TABLE_MAX_PAGES; ++i) {
        table->pages[i] = NULL;
    }
    return table;
}
void free_table(table_t *table)
{
    for (int i = 0; i < TABLE_MAX_PAGES; ++i) {
        free(table->pages[i]);
    }
    free(table);
}

/* 填充 statement*/
/*
 * insert 1 cstack foo@bar.com
*/
sql_cmd_res prepare_statement(input_buffer_t *input_buffer, sql_statement *statement)
{
    if (strncmp(input_buffer->buffer, "insert", 6) == 0) {
        statement->type = STATEMENT_INSERT;
        int args = sscanf(input_buffer->buffer, "insert %d %s %s", &(statement->row_to_insert.id),
                          statement->row_to_insert.user_name,
                          statement->row_to_insert.email);
        if (args < 3) {
            return PREPARE_SYNTAX_ERROR;
        }
        return PREPARE_SUCCESS;
    }
    if (strncmp(input_buffer->buffer, "select", 6) == 0) {
        statement->type = STATEMENT_SELECT;
        return PREPARE_SUCCESS;
    }
    return PREPARE_UNRECOGNIZED_STATEMENT;
}

/*处理源命令*/
meta_cmd_res do_meta_command(input_buffer_t *input_buffer)
{
    if (strcmp(input_buffer->buffer, ".exit") == 0) {
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
sqlite> create table_t users (id int, username varchar(255), email varchar(255));
sqlite> .tables
users
sqlite> .exit
 * @param argc
 * @param argv
 * @return
 */

int main(int argc, char *argv[])
{
    table_t *table = new_table();
    input_buffer_t *input_buffer = new_input_buffer();
    while (true) {
        /*1. Making a Simple REPL*/
        print_prompt();
        read_input(input_buffer);
        if (input_buffer->buffer[0] == '.') {
            // 源命令
            switch (do_meta_command(input_buffer)) {
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
        }

        switch (execute_statement(&statement, table)) {
            case EXECUTE_SUCCESS:
                printf("Executed,\n");
                break;
            case EXECUTE_TABLE_FULL:
                printf("Error : table full. \n");
                break;
        }
    }
    return 0;
}