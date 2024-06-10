#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "signal_handler.h"
#include <errno.h>
#include <pthread.h>
#include <stdarg.h>

#define BUFFER_SIZE 1024

// 全局配置参数
int port = 12345;
int hash_table_size = 100;
int max_cache_size = 1000;

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

void log_message(const char *format, ...)
{
    FILE *logfile = fopen("server.log", "a");
    if (logfile == NULL)
    {
        perror("Could not open log file");
        return;
    }

    va_list args;
    va_start(args, format);
    vfprintf(logfile, format, args);
    va_end(args);

    fprintf(logfile, "\n");
    fclose(logfile);
}

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
    log_message("CONFIG port: %d , max_hash_table_size: %d, max_cache_size: %d\n", port, hash_table_size, max_cache_size);
    fclose(file);
}

void send_response(int socket, const char *response)
{
    char buffer[BUFFER_SIZE];
    snprintf(buffer, BUFFER_SIZE, "%s\n", response);
    send(socket, buffer, strlen(buffer), 0);
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
void delete_lru_entry() {
    // 初始化 oldest 为当前时间之前的一秒
    time_t oldest = time(NULL);
    int oldest_index = -1;
    Entry *oldest_entry = NULL;
    Entry *prev_oldest_entry = NULL;

    // 遍历哈希表查找最老的条目
    for (int i = 0; i < hash_table_size; ++i) {
        Entry *entry = hash_table[i];
        Entry *prev = NULL;
        while (entry) {
            // 记录遍历的条目以便调试
            log_message("Checking entry key: %s, last_accessed: %ld\n", entry->key, entry->last_accessed);
            if (entry->last_accessed <= oldest) {
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
    if (oldest_entry) {
        log_message("Deleting oldest entry - key: %s, value: %s\n", oldest_entry->key, oldest_entry->value);
        if (prev_oldest_entry) {
            prev_oldest_entry->next = oldest_entry->next;
        } else {
            hash_table[oldest_index] = oldest_entry->next;
        }
        free(oldest_entry->key);
        free(oldest_entry->value);
        free(oldest_entry);
        cache_size--;
    } else {
        log_message("No oldest entry found to delete.\n");
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
    log_message("cache_size: %d\n", cache_size);
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
void handle_command(const char *command, int new_socket)
{
    char cmd[BUFFER_SIZE], key[BUFFER_SIZE], value[BUFFER_SIZE];
    int num_args = sscanf(command, "%s %s %s", cmd, key, value);

    if (num_args < 1)
    {
        send_response(new_socket, "ERROR: Invalid command");
        return;
    }

    if (strcmp(cmd, "PUT") == 0)
    {
        if (num_args != 3)
        {
            send_response(new_socket, "ERROR: PUT command requires key and value");
            return;
        }
        put(key, value);
        send_response(new_socket, "OK");
    }
    else if (strcmp(cmd, "GET") == 0)
    {
        if (num_args != 2)
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
        if (num_args != 2)
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
void *client_handler(void *socket_desc)
{
    int new_socket = *(int *)socket_desc;
    char buffer[BUFFER_SIZE] = {0};

    while (!stop)
    {
        // 清空缓冲区
        memset(buffer, 0, BUFFER_SIZE);

        // 读取客户端数据
        int bytes_read = read(new_socket, buffer, BUFFER_SIZE);
        if (bytes_read <= 0)
        {
            // 客户端关闭连接或读取错误
            if (errno == EINTR)
            {
                // 如果 read 被信号中断，重试读取
                log_message("Interrupted read, retrying...\n");
                continue;
            }
            break;
        }

        log_message("Received: %s\n", buffer);

        // 处理命令
        handle_command(buffer, new_socket);

        if (strncmp(buffer, "QUIT", 4) == 0)
        {
            break;
        }
    }

    close(new_socket);
    free(socket_desc);
    pthread_exit(NULL);
}

int main()
{
    int new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    load_config("config.cfg");

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

    // 创建 socket 文件描述符
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // 将 socket 绑定到端口
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // 监听端口
    if (listen(server_fd, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    log_message("Server is listening on port %d\n", port);

    while (!stop)
    {
        // 接受客户端连接
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
        {
            if (stop)
            {
                break; // 如果停止标志被设置，则跳出循环
            }
            perror("accept");
            exit(EXIT_FAILURE);
        }

        pthread_t client_thread;
        int *new_sock = malloc(sizeof(int));
        *new_sock = new_socket;
        if (pthread_create(&client_thread, NULL, client_handler, (void *)new_sock) < 0)
        {
            perror("could not create thread");
            free(new_sock);
            continue;
        }
        pthread_detach(client_thread); // 线程分离，防止内存泄漏
    }

    // 保存数据
    save_data("data.txt");
    // 执行清理操作
    cleanup();
    return 0;
}
