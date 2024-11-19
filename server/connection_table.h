#ifndef CONNECTION_TABLE_H
#define CONNECTION_TABLE_H
#include "array.h"
#include "connection.h"

typedef array_t connection_table;

#define N_MAX_CONNECTIONS (FD_SETSIZE - 3)

void        connection_table_create();
connection_t* connection_table_find_connection_byip(in_addr_t ip);
connection_t* connection_table_get_active_connection(index_t index);
connection_t* connection_table_get_free_connection();
size_t      connection_table_n_active_connections(void);
void        connection_table_remove_connection(connection_t* connection);
#endif

