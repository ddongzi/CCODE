//
// Created by dong on 3/29/24.
//
#include "server.h"
#include "socket.h"
#include "protocol_type.h"
#include "network_config.h"
#include "log.h"
#include "ssl.h"
#include <pthread.h>

void handle_request(void *arg)
{
    socket_fd_t client_socket = (socket_fd_t)(intptr_t)arg;
    uint8_t *buffer = (uint8_t *) malloc(MAX_BUFFER_SIZE);
    if (buffer == NULL) {
        LOG_ERROR("alloc failed.");
        exit(EXIT_FAILURE);
    }
    ssl_body_t *body_ptr ;
    ssl_header_t *header_ptr = (ssl_header_t *) malloc(sizeof(ssl_header_t));
    if (header_ptr == NULL) {
        LOG_ERROR("alloc failed.");
        exit(EXIT_FAILURE);
    }

    size_t size = receive_data(client_socket, buffer, MAX_BUFFER_SIZE);
    size_t header_size = fill_ssl_header(buffer, size, header_ptr);
    body_ptr = (ssl_body_t * ) malloc( sizeof(ssl_body_t)+ header_ptr->len);
    size_t body_size = fill_ssl_body(buffer, size, body_ptr);
    LOG_INFO("header size %zu", header_size);

    close_socket(client_socket);
}

int main(void )
{
    socket_fd_t listen_fd;
    int client_socket;
    listen_fd = create_server_socket(SSL_PORT);
    pthread_t client_thread;

    while (1) {
        client_socket = accept_client(listen_fd);

        if (pthread_create(&client_thread, NULL, (void *)handle_request,(void *)(intptr_t)client_socket) != 0) {
            perror("pthread_create");
            close_socket(client_socket);
        }
    }

    return 0;

}