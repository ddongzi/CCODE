//
// Created by dong on 3/29/24.
//

#ifndef CLIENT_H
#define CLIENT_H
#include <stdint.h>
#include <stdlib.h>
void client_init();
void client_connect(const char *server_address, uint16_t port);
void client_send(const void *data, size_t size);
size_t client_receive(void *buffer, size_t size);
void client_cleanup();
#endif //CLIENT_H
