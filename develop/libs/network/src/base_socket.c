//
// Created by dong on 3/29/24.
//

#include "socket.h"
#include "log.h"
#include <string.h>
// 创建一个套接字, 并且链接
socket_fd_t create_client_socket(uint16_t port)
{
    socket_fd_t sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in address;
    memset(&address, 0, sizeof(address));

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr(LOCAL_IP_ADDR);
    address.sin_port = htons(port);
    int ret;
    ret = connect(sock_fd, (struct sockaddr*)&address, sizeof(address));
    if (ret == -1) {
        return -1;
    }

    return sock_fd;
}


// 创建设置监听套接字
socket_fd_t create_server_socket(uint16_t port)
{
    struct sockaddr_in address;

    socket_fd_t listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd == 0) {
        perror("socket failed");
        return listen_fd;
    }
    // 设置服务器地址和端口
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    // 绑定套接字到指定端口
    if (bind(listen_fd, (struct sockaddr *)&address, sizeof(address))<0) {
        perror("bind failed");
        return listen_fd;
    }
    // 监听端口，最大连接数设置为 5
    if (listen(listen_fd, 5) < 0) {
        perror("listen failed");
        return listen_fd;
    }
    return listen_fd;
}

socket_fd_t accept_client(socket_fd_t listen_fd)
{
    struct sockaddr_in address;
    socklen_t addr_len;
    socket_fd_t new_socket;
    memset(&address, 0, sizeof(address));
    addr_len = 0;
    new_socket = accept(listen_fd, (struct sockaddr *)&address, (socklen_t*)&addr_len);

    return new_socket;
}
void close_socket(socket_fd_t socket)
{
    close(socket);
}
size_t send_data(socket_fd_t socket, const uint8_t *data, size_t size)
{
    return send(socket, data, size, 0);
}
// 从socket读取
size_t receive_data(socket_fd_t socket, uint8_t *buffer, size_t size)
{
    return read(socket, buffer, size);
}
