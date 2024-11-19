#include "message.h"
#include <stdlib.h>
#include <assert.h>
#include "message_manager.h"
// 



// static property
static uint16_t maxBodySize = MAX_MSG_BODY_SIZE_DEFAULT;

/**
 * @brief create a msg with flags and prefix.
 * 
 * @param [in] flags : lifetime
 * @return message_t* 
 */
message_t* message_create(uint16_t flags)
{
    message_t* msg;

    msg = (message_t*)calloc(1, sizeof(message_t) + maxBodySize + 1);
    assert(msg);
    msg->flags = flags;
    msg->prefix = MSG_PREFIX;

    return msg;

}
void message_destroy(message_t* msg)
{
    assert(msg);
    free(msg);
}

connection_t* message_connection(const message_t* msg) { assert(msg); return msg->connection; }
uint16_t message_conn_seqno(const message_t* msg) { assert(msg); return msg->seqno; }
uint16_t message_flags(const message_t* msg) { assert(msg); return msg->flags; }
in_addr_t message_ipaddr(const message_t* msg) { assert(msg); return msg->ipaddr; }
bool message_is_valid_prefix(const message_t* msg) { assert(msg); return msg->prefix == MSG_PREFIX; }
bool message_is_valid_suffix(const message_t* msg) { assert(msg); return (msg->body[msg->size] == MSG_SUFFIX); }
uint16_t message_max_body_size() { return maxBodySize; }
char message_prefix(const message_t* msg) { assert(msg); return msg->prefix; }
uint16_t message_seqno(const message_t* msg) { assert(msg); return msg->seqno; }
uint32_t message_org_id(const message_t* msg) { assert(msg); return msg->original_id; }
uint32_t message_org_seqno(const message_t* msg) { assert(msg); return msg->original_seqno; }
uint16_t message_size(const message_t* msg) { assert(msg); return msg->size; }
uint16_t message_size1() { return MSG_SIZE_PART1; }
uint16_t message_size2(const message_t* msg) { assert(msg); return MSG_SIZE_PART2(msg); }
char message_suffix(message_t* msg) { assert(msg); return *(msg->body + msg->size); }


bool message_is_connection(message_t* msg);

void message_set_max_size(uint16_t size);
void message_set_connection(message_t* msg, const connection_t* connection, in_addr_t ip_addr, uint16_t conn_seqno)
{
    assert(msg);
    msg->connection = connection;
    msg->ipaddr = ip_addr;
    msg->connection_seqno = conn_seqno;
}
void message_set_conn_seqno(message_t* msg, uint16_t conn_seqno)
{
    assert(msg);
    msg->connection_seqno = conn_seqno;
}
void message_set_ipaddr(message_t* msg , in_addr_t ip_addr)
{
    assert(msg);
    msg->ipaddr = ip_addr;
}
void message_set_org_id(message_t* msg , uint32_t org_id)
{
    assert(msg);
    msg->original_id = org_id;
}
void message_set_org_seqno(message_t* msg , uint32_t org_seqno)
{
    assert(msg);
    msg->original_seqno = org_seqno;
}
void message_set_seqno(message_t* msg , uint16_t seqno)
{
    assert(msg);
    msg->seqno = seqno;
}
void message_set_size(message_t* msg, uint16_t size)
{
    assert(msg);
    msg->size = size;
}

char* message_start(message_t* msg)
{
    assert(msg);
    return &msg->prefix;
}