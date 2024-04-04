//
// Created by dong on 3/29/24.
//

// TODO 为啥这里报CMAKE
#include "server.h"
#include "socket.h"
#include "protocol_type.h"
#include "network_config.h"
#include "log.h"
#include "ssl.h"
#include <pthread.h>

void print_ssl(ssl_header_t *header, ssl_body_t * body)
{
    LOG_INFO("SSL Header: Length=%u, Version=%u, Type=%u, Time=%ld, ", header->len, header->version, header->type, (long)header->time);
    LOG_INFO("SSL Body: Length=%u, Data=%s\n", body->size, body->data);
}

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
    int ret = 0;
    while (1) {
        /* code */
        size_t size = receive_data(client_socket, buffer, MAX_BUFFER_SIZE);
        if (size == 0) {
            LOG_INFO("Receive no data.");
            continue;
        }
        LOG_DEBUG("read size %d,  body: len %u ", size, buffer[0]);
        ret = fill_ssl_header(buffer, size, header_ptr);
        if (ret != SUCCESS) {
            LOG_ERROR("fill ssl header failed.");
            continue;
        }
        body_ptr = (ssl_body_t * ) malloc( sizeof(ssl_body_t)+ header_ptr->len);
        fill_ssl_body(buffer + HEADER_LENGTH_BYTE, header_ptr, body_ptr);
        if (ret != SUCCESS) {
            LOG_ERROR("fill ssl body failed.");
            continue;
        }
        print_ssl(header_ptr, body_ptr);
        free(body_ptr);
    }
    free(header_ptr);
    close(client_socket);

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