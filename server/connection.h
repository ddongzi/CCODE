#ifndef CONNECTION_H
#define CONNECTION_H

#include "socket.h"
#include "message.h"
#include "queue.h"
#include <sys/types.h>
#include <netinet/in.h>

typedef size_t index_t;

enum message_lifetime {
    // Permanent lifetime. These messages are created at program startup, and are never destroyed.
    MSG_LIFE_PERM = 0x0001,   
    // Connection lifetime. These messages are created at a connection startup,
    // and are destroyed at a connection shutdown.
    MSG_LIFE_CONN = 0x0002,
    // Discardable.
    MSG_LIFE_DISC = 0x0004,
};

typedef enum {
  CON_ACTIVE_BIT  = 0x0001,
  CON_RECEIVING_BIT  = 0x0002,
  CON_SENDING_BIT  = 0x0004,
  CON_TRACE_ENABLED_BIT  = 0x0008,  // 跟踪连接（如连接性能、连接数据等）
} connection_state_bits;

typedef enum {
  RECV_STATE0 = 1,  // 常规接收
  RECV_STATE1,    // 分段数据 接收p1
  RECV_STATE2,    // 分段数据 接收p2
} recving_state;

typedef enum {
  RECV_PART1_BIT = 0x8000,
  RECV_PART2_BIT = 0x4000,
} recving_state_bits;

typedef struct {
    size_t n_bytes_received;
    size_t n_bytes_to_receive;
    size_t n_bytes_sent;
    size_t n_bytes_to_send;
} bytes_control_t;

typedef struct Message message_t; // 向前声明

typedef struct Connection
{
    int socket;
    connection_state_bits state; // 连接状态

    index_t seqno;  // 标识连接

    // client 
    in_port_t client_port;
    in_addr_t client_ipaddr;
    char client_ipaddr_str[20];

    // 
    index_t index;  // connecttablez中的索引

    // message
    message_t *in_msg;  // 正在recive的msg
    message_t *out_msg; // 正在sent的msg
    queue_t *pending_out_msgs;  // 等待sent消息队列
    index_t last_recvmsg_seqno;

    // current msg 传输状态
    size_t n_bytes_received;
    size_t n_bytes_to_receive;
    size_t n_bytes_sent;
    size_t n_bytes_to_send;

} connection_t;

connection_t* connection_create();
void connection_destroy(connection_t* connection);
void connection_clearAll(connection_t* connection);

void connection_set_inmsg(connection_t* connection, message_t* msg);
void connection_set_outmsg(connection_t* connection, message_t* msg);
void connection_set_pending_outmsgs(connection_t* connection, queue_t* queue);

void connection_set_ipaddr(connection_t* connection, in_addr_t ip);
void connection_set_port(connection_t* connection, in_port_t port);
void connection_set_socket(connection_t* connection, int socket);
void connection_set_index(connection_t* connection, index_t index);

bool connection_in_active(const connection_t* connection);
bool connection_in_sending(const connection_t* connection);
bool connection_in_receiving(const connection_t* connection, recving_state recv_state);
void connection_set_active(connection_t* connection, bool is_active);
void connection_set_sending(connection_t* connection, bool is_sending);
void connection_set_receiving(connection_t* connection, recving_state recv_state, bool is_recving);


index_t connection_index(const  connection_t* connection);
message_t* connection_inmsg(const  connection_t* connection);
in_addr_t connection_ipaddr(const  connection_t* connection);
char*  connection_ipaddr_str(const  connection_t* connection);
index_t connection_lastmsg_seqno( connection_t* connection);
message_t* connection_outmsg(const  connection_t* connection);
queue_t*  connection_pending_outmsgs(const  connection_t* connection);
in_port_t connection_port(const  connection_t* connection);
index_t connection_seqno(const  connection_t* connection);
int   connection_socket(const  connection_t* connection);
uint16_t connection_state(const  connection_t* connection);

bool connection_is_trace_enabled(const connection_t*connection);
void connection_set_trace(connection_t* connection, bool enable);

bytes_control_t* connection_btyes_control(connection_t* connection);

void connection_close(connection_t *connection);

#endif
