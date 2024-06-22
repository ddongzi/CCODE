#ifndef CLUSTER_MANAGER_H
#define CLUSTER_MANAGER_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/md5.h>
#include "cJSON.h"
#include <unistd.h>
#include <stdint.h>
#include "net_utils.h"
#include "log.h"

#define NUM_REPLICAS 3

typedef struct Node {
    char *ip;
    int port;
    char *role;
} Node;

typedef struct HashRing {
    Node **nodes;
    int num_nodes;
} HashRing;

unsigned int hash_function(const char *str) ;
HashRing *create_hash_ring();
Node *get_node(HashRing *ring, const char *key);
void load_cluster_config(HashRing *ring, char *filename) ;
void start_node(Node *node) ;
void start_cluster(HashRing *ring);

void send_command_to_node(Node *node, const char *command);
void forward_to_node(Node *node, const Message *request, int client_socket);
void accept_clients(int listen_fd, HashRing *ring);

#endif // CLUSTER_MANAGER_H