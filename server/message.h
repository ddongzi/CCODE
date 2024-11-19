#ifndef MESSAGE_H
#define MESSAGE_H

#include "connection.h"
#include <sys/types.h>
#include <netinet/in.h>
#include <time.h>
#include <stdbool.h>

#define PACKED __attribute__((packed))

typedef struct Connection connection_t;


typedef struct Message{

    // identification of this connection
    in_addr_t ipaddr;   //  owner of this msg
    uint16_t connection_seqno;  //  seqno of the owner connection
    connection_t* connection;

    // identification of msg
    uint16_t seqno;
    time_t create_time;
    uint16_t flags;

    // msg format
    uint8_t prefix;
    uint32_t original_seqno;    // 消息序列号 TODO ? seqno
    uint32_t original_id;      //   开源id
    uint16_t duration;  // 
    uint16_t size;  // bodysize - 1
    char body[];    // 不定长数组,  最后一个char是suffix 
} PACKED message_t ;


#define MSG_SIZE_PART1  (1 + 4 + 4 + 2 + 2) // 对齐
#define MSG_SIZE_PART2(msg)  ((msg)->size + 1)

#define MAX_MSG_BODY_SIZE_DEFAULT 256
enum {
    MSG_PREFIX = '\x7e', // char : ~, ascii :126
    MSG_SUFFIX = '\x7e'
};

message_t* message_create(uint16_t flags);
void message_destroy(message_t* msg);

connection_t* message_connection(const message_t* msg);
uint16_t message_conn_seqno(const message_t* msg);
uint16_t message_flags(const message_t* msg);
in_addr_t message_ipaddr(const message_t* msg);
bool message_is_valid_prefix(const message_t* msg);
bool message_is_valid_suffix(const message_t* msg);
uint16_t message_max_body_size();
char message_prefix(const message_t* msg);
uint16_t message_seqno(const message_t* msg);
uint32_t message_org_id(const message_t* msg);
uint32_t message_org_seqno(const message_t* msg);
uint16_t message_size(const message_t* msg);
uint16_t message_size1();
uint16_t message_size2(const message_t* msg);
char message_suffix(message_t* msg);

void message_set_max_size(uint16_t size);
void message_set_connection(message_t* msg, const connection_t* connection, in_addr_t ip_addr, uint16_t conn_seqno);
void message_set_conn_seqno(message_t* msg, uint16_t conn_seqno);
void message_set_ipaddr(message_t* msg , in_addr_t ip_addr);
void message_set_org_id(message_t* msg , uint32_t org_id);
void message_set_org_seqno(message_t* msg , uint32_t org_seqno);
void message_set_seqno(message_t* msg , uint16_t seqno);
void message_set_size(message_t* msg, uint16_t size);

void message_copy_connection(message_t* dst, const message_t*  src);

char* message_buffer(message_t* msg);
char* message_start(message_t* msg);
uint8_t message_byte(const message_t* msg, uint16_t msg_offset);
// void message_clone_fields(message_t*  dst, const message_t*  src, ushort n_fields, ...);
// int message_compare_to_message(cchar* src1, const message_t*  src2, ushort msg_offset, ushort size);
// void message_copy_fields(message_t* msg dst, const message_t* msg src, ushort n_fields, ...);
// void message_copy_from_message(char* dst, const message_t* msg src, ushort msg_offset, ushort size);
// void message_copy_to_message(const message_t* msg dst, cchar* src, ushort msg_offset, ushort size);
// void message_fill(const message_t* msg dst, char fill, ushort msg_offset, ushort size);
// void message_set_byte(const message_t* msg dst, ushort msg_offset, uchar byte);

char* message_start(message_t* msg);

#endif