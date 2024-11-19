/**
 * @file connection_table.c
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2024-11-17
 * 
 * @copyright Copyright (c) 2024
 * 
 */

// includes
#include "connection_table.h"
#include <stdbool.h>
#include <sys/select.h>
#include "connection.h"
#include <assert.h>
#include "socket.h"

// private property
static connection_table* connections;
static size_t n_active_connections;

// private functions
/**
 * @brief 
 * 
 * @param [in] connection 
 * @param [in] ipaddr 
 * @return true 
 * @return false 
 */
static bool compare_ipaddr_func(void* connection, void* ipaddr)
{
    return (connection_ipaddr((connection_t*)connection) == *(in_addr_t*)ipaddr);
}


// public interfaces
void connection_table_create()
{
    connection_t* connection;
    connections = array_create(N_MAX_CONNECTIONS);
    for (index_t i = 0; i < N_MAX_CONNECTIONS; i++) {
        //TODO
        connection = connection_create();
        connection_set_index(connection, i);
        array_set(connections, connection, i);
    }
}
connection_t* connection_table_find_connection_byip(in_addr_t ip)
{
    index_t index = array_find(connections, compare_ipaddr_func, &ip);
    return array_get(connections, index);
}
connection_t* connection_table_get_active_connection(index_t index)
{
    // TODO
    if (index > n_active_connections) {
        return NULL;    // 连接已经不active
    }
    connection_t *connection;
    connection = array_get(connections, index);
    assert(index == connection_index(connection));
    return connection;
}
/**
 * @brief 返回一个free connection（未使用，未携带什么信息）
 * 
 * @return connection_t* 
 */
connection_t* connection_table_get_free_connection()
{
    if (n_active_connections >= N_MAX_CONNECTIONS) 
        return NULL;
    return array_get(connections, n_active_connections++);
}
size_t  connection_table_n_active_connections(void)
{
    return n_active_connections;
}
/**
 * @brief 从active中移除connection
 * 
 * @param [in] connection 
 */
void  connection_table_remove_connection(connection_t* connection)
{
    assert(connection);
    assert(n_active_connections);

    index_t index = connection_index(connection);
    assert(index < N_MAX_CONNECTIONS);
    assert(array_get(connections, index) == connection);
    
    //TODO
    // 如果是最后一个active connection,直接去除acitve
    if (index == n_active_connections) {
        n_active_connections--;
        return;
    }

    index_t last = n_active_connections - 1;
    // 否则，将该connection交换到last active
    connection_t* tmp = array_get(connections, last);

    queue_t* saved_pending_outmsgs = connection_pending_outmsgs(connection);
    array_clear(connections, last);
    array_set(connections, connection, last);
    connection_clearAll(connection);
    connection_set_index(connection, last);
    connection_set_pending_outmsgs(connection, saved_pending_outmsgs);

    array_clear(connections, index);
    array_set(connections, tmp, index);
    connection_set_index(tmp, index);

}