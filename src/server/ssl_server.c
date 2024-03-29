//
// Created by dong on 3/29/24.
//
#include "server.h"
#include "socket.h"
#include "protocol_type.h"
#include "network_config.h"
#include "log.h"
#include "ssl.h"

int main(void )
{
    socket_fd_t listen_fd;
    int client_socket;
    listen_fd = create_server_socket(SSL_PORT);
    uint8_t *buffer = (uint8_t *) malloc(MAX_BUFFER_SIZE);

    ssl_body_t *body_ptr ;
    ssl_header_t *header_ptr = (ssl_header_t *) malloc(sizeof(ssl_header_t));
    if (header_ptr == NULL) {
        LOG_ERROR("alloc failed.");
        exit(EXIT_FAILURE);
    }

    if (buffer == NULL) {
        LOG_ERROR("alloc failed.");
        exit(EXIT_FAILURE);
    }

    while (1) {
        client_socket = accept_client(listen_fd);
        size_t size = receive_data(client_socket, buffer, MAX_BUFFER_SIZE);
        size_t header_size = parse_ssl_header(buffer, size, header_ptr);
        body_ptr = (ssl_body_t * ) malloc( sizeof(ssl_body_t)+ header_ptr->len);
        size_t body_size = parse_ssl_body(buffer, size, body_ptr);
        LOG_INFO(SSL_HEADER_TO_STRING(header_ptr));
    }

}