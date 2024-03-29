//
// Created by dong on 3/29/24.
//

#include "socket.h"
#include "log.h"

// 创建设置监听套接字
socket_fd_t create_server_socket(uint16_t port)
{
    struct sockaddr_in address;

    socket_fd_t listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd == 0) {
        LOG_ERROR("socket failed");
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    // 设置服务器地址和端口
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    // 绑定套接字到指定端口
    if (bind(listen_fd, (struct sockaddr *)&address, sizeof(address))<0) {
        LOG_ERROR("bind failed. port (%d).", port);
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    // 监听端口，最大连接数设置为 5
    if (listen(listen_fd, 5) < 0) {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }
    LOG_INFO("Server listening on port (%d)", port);

    return listen_fd;
}

socket_fd_t accept_client(socket_fd_t listen_fd)
{
    struct sockaddr_in address;
    socklen_t addr_len;
    socket_fd_t new_socket;
    if ((new_socket = accept(listen_fd, (struct sockaddr *)&address, (socklen_t*)&addr_len))<0) {
        LOG_ERROR("accept failed.");
        perror("accept failed");
        exit(EXIT_FAILURE);
    }
    return new_socket;
}
void close_socket(socket_fd_t socket)
{
    close(socket);
}
void send_data(socket_fd_t socket, const uint8_t *data, size_t size)
{
    send(socket, data, size, 0);
}
// 从socket读取
size_t receive_data(socket_fd_t socket, uint8_t *buffer, size_t size)
{
    size_t read_num = 0;
    read_num = read(socket, buffer, size);
    return read_num;
}
