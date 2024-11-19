#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "message.h"
#include "log.h"
#include <assert.h>

#define SERVER_IP "127.0.0.1"  // 服务器 IP 地址
#define SERVER_PORT 4000      // 服务器端口号
#define BUFFER_SIZE 1024       // 缓冲区大小

int main() {
    int sockfd;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    ssize_t bytes_sent, bytes_received;


    // 创建 socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // 配置服务器地址
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // 连接到服务器
    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection to the server failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    LOG_INFO("Connected to server at %s:%d\n", SERVER_IP, SERVER_PORT);

    message_t* send_msg;
    message_t recv_msg;
    uint32_t current_seqno = 0;
    size_t size;

    send_msg = message_create(MSG_LIFE_DISC);
    message_set_org_id(send_msg, getpid());
    sprintf(send_msg->body, "%s","hello~");
    
    send_msg->size = 5; 

    // 主循环：发送和接收消息
    while (1) {
        message_set_org_seqno(send_msg, current_seqno++);
        size = message_size1() + message_size2(send_msg);
        // 发送消息
        LOG_DEBUG("suffix %c", *(message_start(send_msg) + size - 1));
        bytes_sent = send(sockfd, message_start(send_msg), size, 0);
        if (bytes_sent < 0) {
            perror("Failed to send message");
            break;
        }
        LOG_INFO("send msg success. size %d orgseqno : %d", size, current_seqno - 1);

        bytes_received = recv(sockfd, buffer, BUFFER_SIZE - 1, 0);
        if (bytes_received < 0) {
            perror("Failed to receive message");
            break;
        } else if (bytes_received == 0) {
            printf("Server closed the connection\n");
            break;
        }
    }

    // 关闭连接
    close(sockfd);
    return 0;
}
