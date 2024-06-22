#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "signal_handler.h"
#include <errno.h>
#include <pthread.h>
#include <stdarg.h>
#include "net_utils.h"
#include "log.h"

#define BUFFER_SIZE 1024

// 全局配置参数
int port = -1;
int hash_table_size = 100;
int max_cache_size = 5;

int is_running = 1;

typedef struct Entry
{
    char *key;
    char *value;
    struct Entry *next;
    time_t last_accessed; // 新增字段，用于记录最后一次访问时间
} Entry;

Entry **hash_table = NULL;
int server_fd = -1;
int cache_size = 0;

void load_config(const char *filename)
{
    FILE *file = fopen(filename, "r");
    if (file == NULL)
    {
        perror("Could not open config file");
        return;
    }

    char line[BUFFER_SIZE];
    while (fgets(line, sizeof(line), file))
    {
        if (strncmp(line, "PORT=", 5) == 0)
        {
            port = atoi(line + 5);
        }
        else if (strncmp(line, "HASH_TABLE_SIZE=", 16) == 0)
        {
            hash_table_size = atoi(line + 16);
        }
        else if (strncmp(line, "MAX_CACHE_SIZE=", 15) == 0)
        {
            max_cache_size = atoi(line + 15);
        }
    }
    log_info("CONFIG port: %d , max_hash_table_size: %d, max_cache_size: %d\n", port, hash_table_size, max_cache_size);
    fclose(file);
}

unsigned int hash(const char *key)
{
    unsigned int hash = 0;
    while (*key)
    {
        hash = (hash << 5) + *key++;
    }
    return hash % hash_table_size;
}
void delete_lru_entry()
{
    // 初始化 oldest 为当前时间之前的一秒
    time_t oldest = time(NULL);
    int oldest_index = -1;
    Entry *oldest_entry = NULL;
    Entry *prev_oldest_entry = NULL;

    // 遍历哈希表查找最老的条目
    for (int i = 0; i < hash_table_size; ++i)
    {
        Entry *entry = hash_table[i];
        Entry *prev = NULL;
        while (entry)
        {
            // 记录遍历的条目以便调试
            log_info("Checking entry key: %s, last_accessed: %ld\n", entry->key, entry->last_accessed);
            if (entry->last_accessed <= oldest)
            {
                oldest = entry->last_accessed;
                oldest_index = i;
                oldest_entry = entry;
                prev_oldest_entry = prev;
            }
            prev = entry;
            entry = entry->next;
        }
    }

    // 如果找到了最老的条目，删除它
    if (oldest_entry)
    {
        log_info("Deleting oldest entry - key: %s, value: %s\n", oldest_entry->key, oldest_entry->value);
        if (prev_oldest_entry)
        {
            prev_oldest_entry->next = oldest_entry->next;
        }
        else
        {
            hash_table[oldest_index] = oldest_entry->next;
        }
        free(oldest_entry->key);
        free(oldest_entry->value);
        free(oldest_entry);
        cache_size--;
    }
    else
    {
        log_info("No oldest entry found to delete.\n");
    }
}

void put(const char *key, const char *value)
{
    unsigned int index = hash(key);

    // 检查是否存在相同的键
    Entry *entry = hash_table[index];
    while (entry != NULL)
    {
        if (strcmp(entry->key, key) == 0)
        {
            // 如果存在相同的键，更新值并返回
            free(entry->value); // 释放旧值内存
            entry->value = strdup(value);
            entry->last_accessed = time(NULL);

            return;
        }
        entry = entry->next;
    }
    // 插入新条目前检查缓存大小
    if (cache_size >= max_cache_size)
    {
        // 删除最少使用的条目
        delete_lru_entry();
    }
    // 如果不存在相同的键，则插入新条目
    Entry *new_entry = (Entry *)malloc(sizeof(Entry));
    new_entry->key = strdup(key);
    new_entry->value = strdup(value);
    new_entry->last_accessed = time(NULL);
    new_entry->next = hash_table[index];
    hash_table[index] = new_entry;
    cache_size++;
}

char *get(const char *key)
{
    unsigned int index = hash(key);
    Entry *entry = hash_table[index];
    while (entry)
    {
        if (strcmp(entry->key, key) == 0)
        {
            entry->last_accessed = time(NULL); // 更新访问时间
            return entry->value;
        }
        entry = entry->next;
    }
    return NULL;
}

void delete(const char *key)
{
    unsigned int index = hash(key);
    Entry *entry = hash_table[index];
    Entry *prev = NULL;

    while (entry)
    {
        if (strcmp(entry->key, key) == 0)
        {
            if (prev)
            {
                prev->next = entry->next;
            }
            else
            {
                hash_table[index] = entry->next;
            }
            free(entry->key);
            free(entry->value);
            free(entry);
            cache_size--;
            return;
        }
        prev = entry;
        entry = entry->next;
    }
}
void update(const char *key, const char *value)
{
    unsigned int index = hash(key);
    Entry *entry = hash_table[index];

    while (entry)
    {
        if (strcmp(entry->key, key) == 0)
        {
            free(entry->value);
            entry->value = strdup(value);
            return;
        }
        entry = entry->next;
    }
    // 如果键不存在，则插入新键值对
    put(key, value);
}
void handle_command(const Message *msg, int new_socket)
{
    char cmd[BUFFER_SIZE], key[BUFFER_SIZE], value[BUFFER_SIZE];
    int num_args = sscanf(msg->command, "%s", cmd);
    if (num_args < 1)
    {
        send_response(new_socket, "ERROR: Invalid command");
        return;
    }

    if (strcmp(cmd, "PUT") == 0)
    {
        int args = sscanf(msg->payload, "%s %s", key, value);
        if (args != 2)
        {
            send_response(new_socket, "ERROR: PUT command requires key and value");
            return;
        }
        put(key, value);
        send_response(new_socket, "OK");
    }
    else if (strcmp(cmd, "GET") == 0)
    {
        int args = sscanf(msg->payload, "%s", key);
        if (args != 1)
        {
            send_response(new_socket, "ERROR: GET command requires key");
            return;
        }
        char *result = get(key);
        if (result)
        {
            send_response(new_socket, result);
        }
        else
        {
            send_response(new_socket, "NOT FOUND");
        }
    }
    else if (strcmp(cmd, "DELETE") == 0)
    {
        int args = sscanf(msg->payload, "%s", key);
        if (args != 1)
        {
            send_response(new_socket, "ERROR: DELETE command requires key");
            return;
        }
        delete (key);
        send_response(new_socket, "DELETED");
    }
    else if (strcmp(cmd, "SELECT") == 0)
    {
        char response[BUFFER_SIZE * 10] = "";
        for (int i = 0; i < hash_table_size; ++i)
        {
            Entry *entry = hash_table[i];
            while (entry)
            {
                strcat(response, entry->key);
                strcat(response, ": ");
                strcat(response, entry->value);
                strcat(response, ";");
                entry = entry->next;
            }
        }
        send_response(new_socket, response);
    }
    else if (strcmp(cmd, "QUIT") == 0)
    {
        send_response(new_socket, "BYE");
    }
    else
    {
        send_response(new_socket, "UNKNOWN COMMAND");
    }
}

void load_data(const char *filename)
{
    FILE *file = fopen(filename, "r");
    if (file == NULL)
    {
        perror("Could not open data file for reading");
        return;
    }

    char key[BUFFER_SIZE], value[BUFFER_SIZE];
    while (fscanf(file, "%s %s\n", key, value) != EOF)
    {
        put(key, value);
    }

    fclose(file);
}
void save_data(const char *filename)
{
    FILE *file = fopen(filename, "w");
    if (file == NULL)
    {
        perror("Could not open data file for writing");
        return;
    }

    for (int i = 0; i < hash_table_size; ++i)
    {
        Entry *entry = hash_table[i];
        while (entry)
        {
            fprintf(file, "%s %s\n", entry->key, entry->value);
            entry = entry->next;
        }
    }

    fclose(file);
}
void cleanup(void)
{
    // 关闭服务器套接字文件描述符
    if (server_fd != -1)
    {
        close(server_fd);
    }

    // 释放哈希表中的内存
    for (int i = 0; i < hash_table_size; ++i)
    {
        Entry *entry = hash_table[i];
        while (entry)
        {
            Entry *temp = entry;
            entry = entry->next;
            free(temp->key);
            free(temp->value);
            free(temp);
        }
    }
}

// 处理客户端请求的线程函数
void *handle_client_command(void *arg)
{
    ThreadArgs *thread_args = (ThreadArgs *)arg;
    int client_socket = thread_args->socket;
    Message *msg = &thread_args->message;

    log_info("Handling client command from %s: %s %s\n", msg->source, msg->command, msg->payload);
    handle_command(msg, client_socket);
    close(client_socket);
}
void start_self()
{
    // Your startup logic here
    is_running = 1;
}
void run_server()
{
    int server_fd = start_server(port);
    int client_socket;
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);

    log_info("Node is listening on port %d\n", port);

    while (is_running)
    {
        if ((client_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen)) < 0)
        {
            if (stop)
            {
                break; // 如果停止标志被设置，则跳出循环
            }
            perror("accept");
            exit(EXIT_FAILURE);
        }

        handle_connection(client_socket);
    }

    close(server_fd);
}
// 处理节点命令
void *handle_node_command(void *arg)
{
    ThreadArgs *thread_args = (ThreadArgs *)arg;
    int node_socket = thread_args->socket;
    Message *msg = &thread_args->message;

    log_info("Handling node command from %s: %s\n", msg->source, msg->command);

    if (strcmp(msg->command, "START") == 0)
    {
        start_self();
        send(node_socket, "STARTED", strlen("STARTED"), 0);
    }
    else if (strcmp(msg->command, "STOP") == 0)
    {
        send(node_socket, "STOPPED", strlen("STOPPED"), 0);
    }
    else if (strcmp(msg->command, "PING") == 0)
    {
        send(node_socket, "PONG", strlen("PONG"), 0);
    }
    else
    {
        send(node_socket, "UNKNOWN NODE COMMAND", strlen("UNKNOWN NODE COMMAND"), 0);
    }

    close(node_socket);
    pthread_exit(NULL);
}
// 处理传入连接
void handle_connection(int new_socket)
{
    ssize_t bytes_received;

    while (1) { // 循环接收消息
        Message msg;
        /* code */
        bytes_received = receive_message(new_socket, &msg);

        if (bytes_received < 0) {
            perror("recv failed");
            close(new_socket);
            return;
        } else if (bytes_received == 0) {
            // 连接被关闭
            printf("Connection closed by peer.\n");
            close(new_socket);
            return;
        }

        ThreadArgs *thread_args = malloc(sizeof(ThreadArgs));
        thread_args->socket = new_socket;
        thread_args->message = msg;

        pthread_t thread;
        if (strcmp(msg.source, "CLIENT") == 0)
        {
            if (pthread_create(&thread, NULL, handle_client_command, thread_args) < 0)
            {
                perror("could not create client thread");
                free(thread_args);
            }
        }
        else if (strcmp(msg.source, "NODE") == 0)
        {
            if (pthread_create(&thread, NULL, handle_node_command, thread_args) < 0)
            {
                perror("could not create node thread");
                free(thread_args);
            }
        }
        else
        {
            log_info("Unknown source: %s\n", msg.source);
            free(thread_args);
        }
    }
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    // load_config("config.cfg");
    port = atoi(argv[1]);

    // 设置信号处理器
    setup_signal_handlers();

    // 初始化哈希表
    hash_table = (Entry **)calloc(hash_table_size, sizeof(Entry *));
    for (int i = 0; i < hash_table_size; ++i)
    {
        hash_table[i] = NULL;
    }
    // 加载数据
    load_data("data.txt");

    // 创建服务器套接字文件描述符
    run_server();

    // 保存数据
    save_data("data.txt");
    // 执行清理操作
    cleanup();
    return 0;
}
