#ifndef SOCKET_H
#define SOCKET_H

#include <netinet/in.h>
#include <sys/socket.h>
#include <stdbool.h>
#include <unistd.h>

void socket_set_nonblocking(int socket, bool nonblocking);
void socket_add_socket_for_reading(int socket);
void socket_add_socket_for_writing(int socket);
void socket_remove_socket_for_reading(int socket);
void socket_remove_socket_for_writing(int socket);

bool socket_connect(int socket, in_addr_t ip, in_port_t port);
int socket_create_client_socket();
int socket_create_server_socket(in_port_t port);
void socket_init();
void socket_ipaddr_to_str(in_addr_t ip, char* str);
bool socket_is_ready_for_reading(int socket);
bool socket_is_ready_for_writing(int socket);
int socket_recv(int socket, char* buffer, size_t length);
int socket_send(int socket, char* buffer, size_t length);
void socket_create_socketpair(int* sender, int* receiver);
int socket_create_worker_socket(int server_socket, in_addr_t* client_ipaddr, in_port_t* client_port);
int socket_wait_events();
int socket_close(int socket);
#endif