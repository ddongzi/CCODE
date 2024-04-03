//
// Created by dong on 3/29/24.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>

#define SERVER_IP "www.baidu.com"
#define SERVER_PORT 80
#define HTTP_REQUEST "GET / HTTP/1.1\r\nHost: www.baidu.com\r\n\r\n"

int main() {
    int sockfd;
    struct sockaddr_in server_addr;
    struct hostent *server;
    char buffer[1024] = {0};

    // 创建套接字
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // 获取服务器信息
    server = gethostbyname(SERVER_IP);
    if (server == NULL) {
        perror("gethostbyname");
        exit(EXIT_FAILURE);
    }

    // 设置服务器地址结构
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    memcpy(&server_addr.sin_addr.s_addr, server->h_addr, server->h_length);
    server_addr.sin_port = htons(SERVER_PORT);

    // 连接到服务器
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect");
        exit(EXIT_FAILURE);
    }

    // 发送 HTTP 请求
    if (send(sockfd, HTTP_REQUEST, strlen(HTTP_REQUEST), 0) < 0) {
        perror("send");
        exit(EXIT_FAILURE);
    }

    // 接收服务器响应并打印
    ssize_t bytes_received;
    while ((bytes_received = recv(sockfd, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[bytes_received] = '\0';
        printf("%s", buffer);
    }

    if (bytes_received < 0) {
        perror("recv");
        exit(EXIT_FAILURE);
    }

    // 关闭套接字
    close(sockfd);

    return 0;
}
