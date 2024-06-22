#include "cluster_manager.h"

int port = -1;

/**/
unsigned int hash_function(const char *str) {
    unsigned char digest[MD5_DIGEST_LENGTH];
    MD5((unsigned char*)str, strlen(str), digest);
    unsigned int hash = 0;
    for (int i = 0; i < 4; i++) {
        hash = (hash << 8) | digest[i];
    }
    return hash;
}
HashRing *create_hash_ring() {
    HashRing *ring = (HashRing*)malloc(sizeof(HashRing));
    load_cluster_config(ring, "cluster.json");
    return ring;
}
Node *get_node(HashRing *ring, const char *key) {
    unsigned int hash = hash_function(key);
    int node_index = hash % ring->num_nodes;
    return ring->nodes[node_index];
}

void load_cluster_config(HashRing *ring, char *filename) {
    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        log_error("Error opening file %s\n", filename);
        exit(1);
    }
    
    fseek(fp, 0L, SEEK_END);
    long length = ftell(fp);
    fseek(fp, 0L, SEEK_SET);
    
    char *data = (char*)malloc(length + 1);
    fread(data, 1, length, fp);
    data[length] = '\0';
    fclose(fp);
    
    cJSON *root = cJSON_Parse(data);
    if (root == NULL) {
        log_error("Error before: %s\n", cJSON_GetErrorPtr());
        exit(EXIT_FAILURE);
    }
    
    cJSON *nodes = cJSON_GetObjectItem(root, "nodes");
    ring->num_nodes = cJSON_GetArraySize(nodes);
    ring->nodes = (Node**)malloc(ring->num_nodes * sizeof(Node*));
    
    for (int i = 0; i < ring->num_nodes; i++) {
        cJSON *node = cJSON_GetArrayItem(nodes, i);
        cJSON *ip = cJSON_GetObjectItem(node, "ip");
        cJSON *port = cJSON_GetObjectItem(node, "port");
        cJSON *role = cJSON_GetObjectItem(node, "role");
        
        ring->nodes[i] = (Node*)malloc(sizeof(Node));
        ring->nodes[i]->ip = (char*)malloc(strlen(ip->valuestring) + 1);
        strcpy(ring->nodes[i]->ip, ip->valuestring);
        ring->nodes[i]->port = port->valueint;
        ring->nodes[i]->role = (char*)malloc(strlen(role->valuestring) + 1);
        strcpy(ring->nodes[i]->role, role->valuestring);
    }
    
    cJSON_Delete(root);
    free(data);
}
void start_node(Node *node) 
{
    // 节点自启动
    send_command_to_node(node, "START");
}

void start_cluster(HashRing *ring) 
{
    int listen_fd = start_server(port);
    log_info("Starting cluster...");
    for (int i = 0; i < ring->num_nodes; i++) {
        if (ring->nodes[i] && ring->nodes[i]->port != port) {
            start_node(ring->nodes[i]);
            log_info("Started node %s:%d\n", ring->nodes[i]->ip, ring->nodes[i]->port);
        }
    }
    log_info("Cluster started  go accept\n");
    accept_clients(listen_fd, ring);
}

void send_command_to_node(Node *node, const char *command) {
    int sockfd = connect_to_server(node->ip, node->port);
    if (sockfd < 0) {
        log_error( "Failed to connect to node %s:%d\n", node->ip, node->port);
        return;
    }
    Message msg = {
        .source = "NODE",
        .command = command,
    };
    send_message(sockfd, &msg);
    
    Message response;
    receive_message(sockfd, &response);
    log_info("Received response from node %s:%d\n", node->ip, node->port);
    close(sockfd);
}

void forward_to_node(Node *node, const Message *request, int client_socket) {
    int node_socket = connect_to_server(node->ip, node->port);
    if (node_socket < 0) {
        log_error("Failed to connect to node %s:%d\n", node->ip, node->port);
        return;
    }

    // 发送请求到数据节点
    if (send_message(node_socket, request) < 0) {
        log_error("Failed to send message to node %s:%d\n", node->ip, node->port);
        close(node_socket);
        return;
    }

    // 接收数据节点的响应
    Message response;
    if (receive_message(node_socket, &response) < 0) {
        log_error("Failed to receive response from node %s:%d\n", node->ip, node->port);
    } else {
        // 将响应转发回客户端
        if (send_message(client_socket, &response) < 0) {
            log_error("Failed to forward response to client\n");
        }
    }

    close(node_socket);
}

void accept_clients(int listen_fd, HashRing *ring) {

    log_info("Accepting clients...");
    while (1) {
        log_info("Waiting for client request...");
        int client_socket = accept_connection(listen_fd);
        if (client_socket < 0) {
            log_error("Failed to accept client connection\n");
            continue;
        }
        
        // 接收客户端请求
        Message msg;

        // 根据请求的键确定应该发送到哪个节点处理
        Node *node = get_node(ring, msg.command);
        log_info("Forwarding request to node %s:%d\n", node->ip, node->port);
        // 将请求转发给相应节点处理
        forward_to_node(node, &msg, client_socket);

        close(client_socket);
    }

    close(listen_fd);
}


int main(int argc, char const *argv[])
{
    HashRing *ring = create_hash_ring();
    port = atoi(argv[1]);
    start_cluster(ring);

    while (1) {
        sleep(1);
    }
    return 0;
}
