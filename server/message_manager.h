#ifndef MESSAGE_MANAGER_H
#define MESSAGE_MANAGER_H

#include <stdint.h>
#include <stdlib.h>
#include "message_queue.h"

void message_manager_init();
void message_manager_create_conn_msgs();

message_t * message_manager_get_free_msg();

void message_manager_dispose_msg(message_t* msg);
size_t message_manager_destroy_msgs(message_queue_t* queue, size_t nmsgs, uint16_t flags);
void message_manager_add_inmsg(const message_t* msg);

#endif