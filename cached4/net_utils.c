// net_utils.c
#include "net_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "log.h"

// 启动服务器
int start_server(int port) {
    int server_fd;
    struct sockaddr_in address;
    int opt = 1;

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("Setsockopt failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("Listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    return server_fd;
}

// 连接到服务器
int connect_to_server(const char *ip, int port) {
    int sockfd;
    struct sockaddr_in server_addr;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation failed");
        return -1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, ip, &server_addr.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        close(sockfd);
        return -1;
    }

    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        close(sockfd);
        return -1;
    }

    return sockfd;
}

// 发送消息
// 发送消息
ssize_t send_message(int socket, const Message *msg) {
    char buffer[sizeof(Message)];
    serialize_message(buffer, msg);
    return send(socket, buffer, sizeof(buffer), 0);
}

// 接收消息
ssize_t receive_message(int socket, Message *msg) {
    char buffer[sizeof(Message)];
    ssize_t bytes_received = recv(socket, buffer, sizeof(buffer), 0);
    if (bytes_received > 0) {
        deserialize_message(buffer, msg);
    }
    return bytes_received;
}



int accept_connection(int server_socket) {
    struct sockaddr_in client_addr;
    int client_socket, client_addr_len;

    // 等待客户端连接
    client_socket = accept(server_socket, (struct sockaddr *)&client_addr, (socklen_t *)&client_addr_len);
    if (client_socket < 0) {
        perror("Error accepting connection");
    }
    return client_socket;
}
void send_response(int socket, const char *response)
{
    char buffer[BUFFER_SIZE];
    snprintf(buffer, BUFFER_SIZE, "%s\n", response);
    send(socket, buffer, strlen(buffer), 0);
}
// 序列化消息
void serialize_message(char *buffer, Message *msg) {
    print_buffer_hex_and_ascii(buffer, 30);
    snprintf(buffer, BUFFER_SIZE, "%s:%s:%s", msg->source, msg->command, msg->payload);
}

// 反序列化消息// 去除字符串末尾空白字符的函数
void trim_trailing_whitespace(char *str) {
    int n = strlen(str) - 1;
    while (n >= 0 && isspace((unsigned char)str[n])) {
        str[n] = '\0';
        n--;
    }
}

void deserialize_message(char *buffer, Message *msg) {
    // 使用 strtok 分割字符串，并确保不会超过每个字段的最大长度
    print_buffer_hex_and_ascii(buffer, 30);
    char *token = strtok(buffer, " ");
    if (token != NULL) {
        strncpy(msg->source, token, sizeof(msg->source) - 1);
        msg->source[sizeof(msg->source) - 1] = '\0';
    } else {
        msg->source[0] = '\0';
    }

    token = strtok(NULL, " ");
    if (token != NULL) {
        strncpy(msg->command, token, sizeof(msg->command) - 1);
        msg->command[sizeof(msg->command) - 1] = '\0';
    } else {
        msg->command[0] = '\0';
    }

    token = strtok(NULL, "");
    if (token != NULL) {
        strncpy(msg->payload, token, sizeof(msg->payload) - 1);
        msg->payload[sizeof(msg->payload) - 1] = '\0';
    } else {
        msg->payload[0] = '\0';
    }

    // 去除每个字段末尾的空白字符
    trim_trailing_whitespace(msg->source);
    trim_trailing_whitespace(msg->command);
    trim_trailing_whitespace(msg->payload);
    log_info("deserialized message: source=%s, command=%s, payload=%s", msg->source, msg->command, msg->payload);
}

int read_n_bytes(int socket, char *buffer, int n) {
    int total_bytes_read = 0;
    while (total_bytes_read < n) {
        int bytes_read = recv(socket, buffer + total_bytes_read, n - total_bytes_read, 0);
        if (bytes_read <= 0) {
            return bytes_read; // 读取错误或连接关闭
        }
        total_bytes_read += bytes_read;
    }
    return total_bytes_read;
}



void print_buffer_hex_and_ascii(const unsigned char *buffer, size_t length) {
    // 每行最多输出16个字节，因此我们需要足够大的缓冲区来存储整行
    // 8字符的偏移量 + 16 * 3字符的十六进制数 + 空格 + 16字符的ASCII字符 + 结束符
    char line[8 + 16 * 3 + 1 + 16 + 1 + 1]; // 额外的1是为结束符'\0'准备的

    for (size_t i = 0; i < length; i += 16) {
        int offset = 0;
        // 打印行号
        offset += sprintf(line + offset, "%08x  ", (unsigned int)i);

        // 打印每行的16进制值
        for (size_t j = 0; j < 16; j++) {
            if (i + j < length) {
                offset += sprintf(line + offset, "%02x ", buffer[i + j]);
            } else {
                offset += sprintf(line + offset, "   ");
            }
            // 每8字节加一个空格，增加可读性
            if (j == 7) {
                offset += sprintf(line + offset, " ");
            }
        }

        // 打印对应的ASCII字符
        offset += sprintf(line + offset, " |");
        for (size_t j = 0; j < 16; j++) {
            if (i + j < length) {
                unsigned char c = buffer[i + j];
                if (isprint(c)) {
                    offset += sprintf(line + offset, "%c", c);
                } else {
                    offset += sprintf(line + offset, ".");
                }
            } else {
                offset += sprintf(line + offset, " ");
            }
        }
        sprintf(line + offset, "|");
        
        // 打印整行
        log_info("BUFFER VIEW %s", line);
    }
}