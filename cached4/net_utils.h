// net_utils.h
#ifndef NET_UTILS_H
#define NET_UTILS_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <ctype.h>

#define BUFFER_SIZE 1024


typedef struct {
    char source[10];
    char command[10];
    char payload[BUFFER_SIZE];
} Message;

#define M

typedef struct {
    int socket;
    Message message;
} ThreadArgs;


// 定义服务器和客户端通用的函数
int start_server(int port);
int connect_to_server(const char *ip, int port);
int accept_connection(int server_socket) ;
ssize_t send_message(int socket, const Message *message);
ssize_t receive_message(int socket, Message *message);

void send_response(int socket, const char *response);

int read_n_bytes(int socket, char *buffer, int n) ;
void serialize_message(char *buffer, Message *msg);
void deserialize_message(char *buffer, Message *msg);


void print_buffer_hex_and_ascii(const unsigned char *buffer, size_t length) ;


#endif // NET_UTILS_H
