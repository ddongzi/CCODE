//
// Created by dong on 3/29/24.
//

#ifndef NETWORK_SOCKET_H
#define NETWORK_SOCKET_H
#include <stdint.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "network_config.h"
typedef int socket_fd_t;

void socket_init();
// 客户端
socket_fd_t create_client_socket(uint16_t port);

// 服务端
socket_fd_t create_server_socket(uint16_t port);
socket_fd_t accept_client(socket_fd_t listen_fd);
void close_socket(socket_fd_t socket);
size_t send_data(socket_fd_t socket, const uint8_t *data, size_t size);
size_t receive_data(socket_fd_t socket, uint8_t *buffer, size_t buffer_size);

#endif //NETWORK_SOCKET_H
