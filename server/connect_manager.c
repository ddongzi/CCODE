#include "queue.h"
#include "connect_manager.h"
#include "socket.h"
#include "connection_table.h"
#include "message_manager.h"
#include "log.h"
#include <assert.h>

// define
#define DEFAULT_TCP_SERVICE_PORT 4000

// static property
static int listening_socket;
static in_port_t service_port = DEFAULT_TCP_SERVICE_PORT;
static int ipc_recver_socket;
static int ipc_sender_socket;

static queue_t* close_conn_queue;   // 待关闭的连接队列

// static helper functions

/**
 * @brief 
 * 
 * @param [in] old_connection 
 */
static void closeOldConnection(connection_t * old_connection)
{

}

/**
 * @brief fill the free connection
 * 
 * @param [in] connection 
 * @param [in] socket 
 * @param [in] ip 
 * @param [in] port 
 */
static void prepare_new_connection(connection_t* connection, int socket, in_addr_t ip, in_port_t port)
{
    connection_set_socket(connection, socket);
    connection_set_port(connection, port);
    connection_set_ipaddr(connection, ip);
    // TODO 回调函数， 跟踪

    socket_ipaddr_to_str(ip, connection_ipaddr_str(connection));
    LOG_INFO("new connection with [IP:%s port:%d socket:%d seq:%d]",
        connection_ipaddr_str(connection), port, socket, connection_seqno(connection)
        );
    LOG_INFO("number of active connections now: %d", connection_table_n_active_connections());
}

/**
 * @brief 从一个new connection 开始receive
 * 
 * @param [in] connection 
 */
static void start_receive_msg(connection_t* connection)
{
    message_t* msg;
    if (!(msg = connection_inmsg(connection))) {
        LOG_INFO("Connection don't have inmsg .requesting a new free input message");
        msg = message_manager_get_free_msg();
        connection_set_inmsg(connection, msg);
    }    
    // message ~ connection
    message_set_connection(msg, connection, connection_ipaddr(connection), connection_seqno(connection));
    message_set_seqno(msg, connection_lastmsg_seqno(connection));

    // TODO state??
    // 
    connection_set_receiving(connection, RECV_STATE0, true);
    connection_set_receiving(connection, RECV_STATE1, true);
    connection_btyes_control(connection)->n_bytes_received = 0;
    connection_btyes_control(connection)->n_bytes_to_receive = message_size1();

    // 
    LOG_INFO("starting monitoring the socket %d for read events", connection->socket);
    socket_add_socket_for_reading(connection_socket(connection));
}

/**
 * @brief 收到remote连接请求。
 *  1. accept
 * 
 */
static void process_new_connection()
{
    int worker_socket;
    in_addr_t client_ip;
    in_addr_t client_port;
    connection_t  *new_connection;

    worker_socket = socket_create_worker_socket(listening_socket, &client_ip, &client_port);
    if (worker_socket == -1) {
        LOG_ERROR("failed accept of new connection");
        return;
    }
    socket_set_nonblocking(worker_socket, true);

    // TODO reuse
    if (!(new_connection = connection_table_get_free_connection())) {
        LOG_ERROR("run out of connections!!!");
        socket_close(worker_socket);
        return;
    }
    prepare_new_connection(new_connection, worker_socket, client_ip, client_port);
    LOG_INFO("creating connection messages for new connection");
    message_manager_create_conn_msgs();
    // TODO
    LOG_INFO("starting receiving messages for the new connection");
    start_receive_msg(new_connection);
}
/**
 * @brief analyzes the firset part of the received msg;
 * 
 * @param [in] connection 
 */
static void process_msg_part1(connection_t *connection)
{
    message_t* msg;
    connection_set_receiving(connection, RECV_STATE1, false);
    
    msg = connection_inmsg(connection);
    assert(msg);
    LOG_INFO("processing msg#%d part1", msg->original_seqno);

    if(!message_is_valid_prefix(msg)) {
        LOG_ERROR("invalid msg prefix");
        connection_close(connection);
        return;
    }
    if (message_size(msg) > message_max_body_size()) {
        LOG_ERROR("invalid msg body size, too big");
        connection_close(connection);
        return;
    }
    connection_set_receiving(connection, RECV_STATE2, true);
    connection_btyes_control(connection)->n_bytes_to_receive = message_size2(msg);
}
/**
 * @brief Analyzes the second part of a received Message. 
 * 
 * @param [in] connection 
 */
static void process_msg_part2(connection_t* connection)
{
    message_t* msg;

    msg = connection_inmsg(connection);
    assert(msg);
    LOG_INFO("processing msg#%d part2", msg->original_seqno);
    
    connection_set_receiving(connection, RECV_STATE2, false);

    if (!message_is_valid_suffix(msg)) {
        LOG_ERROR("invalid suffix %c,  size : %d, bodyf: %c", msg->body[msg->size], msg->size, msg->body[0]);
        connection_close(connection);
        return;
    }
    LOG_INFO("adding this input msg#%d into message manager.", msg->original_seqno);
    message_manager_add_inmsg(msg);

    // make a new free msg to connection
    msg = message_manager_get_free_msg();
    connection_set_inmsg(connection, msg);
    message_set_connection(msg, connection, connection_ipaddr(connection), connection_seqno(connection));
    connection_set_receiving(connection, RECV_STATE1, true); 
    // ?? 不要设置state0吗
    connection_btyes_control(connection)->n_bytes_received = 0;
    connection_btyes_control(connection)->n_bytes_to_receive = message_size1();
}

/**
 * @brief receive part msg for reading event
 * 
 * @param [in] connection 
 */
static void receive_partial_msg(connection_t* connection)
{
    size_t length;
    message_t* msg;
    bytes_control_t* bytes_ctrl = connection_btyes_control(connection);
    if (!connection_in_receiving(connection, RECV_STATE0)) {
        LOG_ERROR("unexpected.");
        connection_close(connection);
        return;        
    }
    msg = connection_inmsg(connection);
    length = socket_recv(
        connection_socket(connection),
        message_start(msg) + bytes_ctrl->n_bytes_received,
        bytes_ctrl->n_bytes_to_receive
    );
    if (length != bytes_ctrl->n_bytes_to_receive) {
        LOG_INFO("TODO, length decide behavior. maybe retry. length %d, expect %d",
            length, bytes_ctrl->n_bytes_to_receive
        );
    }
    bytes_ctrl->n_bytes_to_receive -= length;
    bytes_ctrl->n_bytes_received += length;
    if (bytes_ctrl->n_bytes_to_receive) {
        LOG_INFO("received %d bytes is not enough. need to continue receive %d bytes", 
            length, bytes_ctrl->n_bytes_to_receive
        );
        return;
    }
    
    // 上面收到header部分
    if (connection_in_receiving(connection, RECV_STATE1)) {
        process_msg_part1(connection);   
    } else if (connection_in_receiving(connection, RECV_STATE2)) {
        process_msg_part2(connection);
    } else {
        LOG_ERROR("unexpected state in receive msg");
        connection_close(connection);
    }
    
}

/**
 * @brief process the connection event , which is in connection table.
 * 
 * @param [in] nevents 
 */
static void process_connection_table(int nevents)
{
    size_t nactive_connections;
    connection_t* connection;
    int socket;
    char info[50];

    nactive_connections = connection_table_n_active_connections();
    LOG_INFO("checking for read/write events, for %d active connections", nactive_connections);
    for (index_t i = 0; i < nactive_connections; i++) {
        connection = connection_table_get_active_connection(i);
        assert(connection);

        socket = connection_socket(connection);
        sprintf(info, "[IP:%s port:%d socket:%d seq:%d]", connection_ipaddr_str(connection),
            connection_port(connection), socket, connection_seqno(connection)
            );
        LOG_INFO("connection table active infos. %s", info);

        if (socket_is_ready_for_reading(socket)) {
            
            receive_partial_msg(connection);
            // todo
            
        }

    }
    
}

// public interfaces
/**
 * @brief 
 *  1. 创建listen和ipc socket，添加到对应fdset
 *  2. 创建connection table,
 *  3. 创建close connection queue
 */
void connect_manager_init()
{
    LOG_INFO("listen socket create.");
    listening_socket = socket_create_server_socket(service_port);
    socket_set_nonblocking(listening_socket, true);
    LOG_DEBUG("listen socket %u",listening_socket);
    
    LOG_INFO("listen socket start reading");
    socket_add_socket_for_reading(listening_socket);

    LOG_INFO("creating the IPC socket pair");
    socket_create_socketpair(&ipc_sender_socket, &ipc_recver_socket);
    socket_set_nonblocking(ipc_recver_socket, true);

    LOG_INFO("starting monitoring the IPC receiver socket");
    socket_add_socket_for_reading(ipc_recver_socket);

    LOG_INFO("creating the connection table");
    connection_table_create();

    LOG_INFO("creating the close connection queue");
    close_conn_queue = queue_create(5);

    // todo
        
}

void connect_manager_run()
{
    LOG_INFO("starting running the connection manager");
    int nevents;    // 
    for (;;) {
        // LOG_DEBUG("waiting for events");
        nevents = socket_wait_events();  
        // LOG_DEBUG("found %d events", nevents);

        // remote client is requesting
        if (socket_is_ready_for_reading(listening_socket)) {
            LOG_INFO("found new remote connection event");
            process_new_connection();
            nevents--;
        }

        // local client send a new msg
        if (nevents > 0 && socket_is_ready_for_reading(ipc_recver_socket)) {
            LOG_INFO("found local IPC event");
            nevents--;
        }
        if (nevents > 0) {
            LOG_INFO("the normal case. worker socket read/write");
            process_connection_table(nevents);
        }
    }
    

}