//
// Created by dong on 4/25/24.
//

/*1. Making a Simple REPL*/
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

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
    PREPARE_UNRECOGNIZED_STATEMENT
} sql_cmd_res;

/*sql 命令类型*/
typedef enum {
    STATEMENT_INSERT,
    STATEMENT_SELECT
} statement_type;

typedef struct {
    statement_type type;
} sql_statement;

void execute_statement(sql_statement *statement)
{
    switch (statement->type) {
        case STATEMENT_INSERT:
            printf("This is where we would do an insert.\n");
            break;
        case STATEMENT_SELECT:
            printf("This is where we would do a select.\n");
            break;
    }
}

/* 填充 statement*/
sql_cmd_res prepare_statement(input_buffer_t *input_buffer, sql_statement *statement)
{
    if (strncmp(input_buffer->buffer, "insert", 6) == 0) {
        statement->type = STATEMENT_INSERT;
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
    input_buffer_t *input_buffer = new_input_buffer();
    while (true) {
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
            case PREPARE_UNRECOGNIZED_STATEMENT:
                printf("Unrecognized SQL command `%s` \n", input_buffer->buffer);
                continue;
        }

        execute_statement(&statement);
        printf("Executed. \n");
    }
    return 0;
}