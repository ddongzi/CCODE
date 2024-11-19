
#include "connection.h"
#include <assert.h>
#include <string.h>
#include "log.h"
#include "message_manager.h"

// 


// static property
static index_t current_seqno;





// static functions
/**
 * @brief 
 * 
 * @param [in] connection 
 * @param [in] normal_close 
 */
static void close_connection_ex(connection_t* connection, uint8_t normal_close)
{
    int socket = connection_socket(connection);
    message_t* in_msg = connection_inmsg(connection);
    message_t* out_msg = connection_outmsg(connection);

    LOG_WARN("closing %sconnection [IP:%s port:%d socket:%d seq:%d]", 
        connection_ipaddr_str(connection), connection_port(connection), socket, connection_seqno(connection)
        );

    socket_close(socket);

    socket_remove_socket_for_reading(socket);
    socket_remove_socket_for_writing(socket);

    if (in_msg) {
        message_manager_dispose_msg(in_msg);
        connection_set_inmsg(connection, NULL);
    }
    if (out_msg) {
        message_manager_dispose_msg(out_msg);
        connection_set_outmsg(connection, NULL);
    }

}


// public interfaces

/**
 * @brief 创建connection，分配内存
 * 
 * @return connection_t* 
 */
connection_t* connection_create()
{
    connection_t* connection = (connection_t*)calloc(1, sizeof(connection_t));
    assert(connection);

    connection->pending_out_msgs = queue_create(3);
    connection->seqno = current_seqno++;
    return connection;
}
void connection_destroy(connection_t* connection)
{
    assert(connection);
    assert(!connection->in_msg && !connection->out_msg);
    
    assert(queue_isempty(connection->pending_out_msgs));
    queue_destroy(connection->pending_out_msgs);
    free(connection);
}
/**
 * @brief 清空connection内容，置0
 * 
 * @param [in] connection 
 */
void connection_clearAll(connection_t* connection)
{
    assert(connection);
    memset(connection, 0, sizeof(connection));
}

void connection_set_inmsg(connection_t* connection, message_t* msg)
{
    assert(connection);
    connection->in_msg = msg;
}
void connection_set_outmsg(connection_t* connection, message_t* msg)
{
    assert(connection);
    connection->out_msg = msg;
}
void connection_set_pending_outmsgs(connection_t* connection, queue_t* queue)
{
    connection->pending_out_msgs = queue;
}
void connection_set_ipaddr(connection_t* connection, in_addr_t ip)
{
    assert(connection);
    connection->client_ipaddr = ip;
}
void connection_set_port(connection_t* connection, in_port_t port)
{
    assert(connection);
    connection->client_port = port;
}
void connection_set_socket(connection_t* connection, int socket)
{
    assert(connection);
    connection->socket = socket;
}
void connection_set_index(connection_t* connection, index_t index)
{
    assert(connection);
    connection->index = index;
}
bool connection_in_active(const connection_t* connection)
{
    assert(connection);
    return (connection->state & CON_ACTIVE_BIT);
}
bool connection_in_sending(const connection_t* connection)
{
    assert(connection);
    return (connection->state & CON_SENDING_BIT);
}
bool connection_in_receiving(const connection_t* connection, recving_state recv_state)
{
    assert(connection);
    if (recv_state == RECV_STATE0)
        return (connection->state & CON_RECEIVING_BIT);
    if (recv_state == RECV_STATE1)
        return (connection->state & RECV_PART1_BIT);
    
    return (connection->state & RECV_PART2_BIT);
}
void connection_set_active(connection_t* connection, bool is_active)
{
    assert(connection);
    if (is_active) 
        connection->state |= CON_ACTIVE_BIT;
    else 
        connection->state &= ~CON_ACTIVE_BIT;
}
void connection_set_sending(connection_t* connection, bool is_sending)
{
    assert(connection);
    if (is_sending) 
        connection->state |= CON_SENDING_BIT;
    else 
        connection->state &= ~CON_SENDING_BIT;
}
/**
 * @brief set receive state.
 * 
 * @param [in] connection 
 * @param [in] recv_state 
 * @param [in] is_recving  | true:open recv state. | false : close recv
 */
void connection_set_receiving(connection_t* connection, recving_state recv_state, bool is_recving)
{
    assert(connection);
    if (is_recving) {
        if (recv_state == RECV_STATE0)
            connection->state |= CON_RECEIVING_BIT;
        else if(recv_state == RECV_STATE1)
            connection->state |= RECV_PART1_BIT;
        else
            connection->state |= RECV_PART2_BIT;
    } else {
        // 关闭recv state
        if (recv_state == RECV_STATE0)
            connection->state &= ~CON_RECEIVING_BIT;
        else if(recv_state == RECV_STATE1)
            connection->state &= ~RECV_PART1_BIT;
        else
            connection->state &= ~RECV_PART2_BIT;
    }
}

index_t connection_index(const  connection_t* connection)
{
    assert(connection);
    return connection->index;
}
message_t* connection_inmsg(const  connection_t* connection)
{
    assert(connection);
    return connection->in_msg;
}
in_addr_t connection_ipaddr(const  connection_t* connection)
{
    assert(connection);
    return connection->client_ipaddr; 
}
char*  connection_ipaddr_str(const  connection_t* connection)
{
    assert(connection);
    return connection->client_ipaddr_str;  
}
index_t connection_lastmsg_seqno( connection_t* connection)
{
    assert(connection);
    return connection->last_recvmsg_seqno;  
}
message_t* connection_outmsg(const  connection_t* connection)
{
    assert(connection);
    return connection->out_msg;  
}
queue_t*  connection_pending_outmsgs(const  connection_t* connection)
{
    assert(connection);
    return connection->pending_out_msgs;      
}
in_port_t connection_port(const  connection_t* connection)
{
    assert(connection);
    return connection->client_port;      
}
index_t connection_seqno(const  connection_t* connection)
{
    assert(connection);
    return connection->seqno;       
}
int   connection_socket(const  connection_t* connection)
{
    assert(connection);
    return connection->socket;       
}
uint16_t connection_state(const  connection_t* connection)
{
    assert(connection);
    return connection->state;     
}
bool connection_is_trace_enabled(const connection_t*connection)
{
    assert(connection);
    return (connection->state & CON_TRACE_ENABLED_BIT);         
}
void connection_set_trace(connection_t* connection, bool enable)
{
    assert(connection);
    if (enable)
        connection->state |= CON_TRACE_ENABLED_BIT;
    else
        connection->state &= ~ CON_TRACE_ENABLED_BIT;
}
bytes_control_t* connection_btyes_control(connection_t* connection)
{
    return (bytes_control_t*)(&connection->n_bytes_received);
}

void connection_close(connection_t *connection)
{

}
