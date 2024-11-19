#ifndef MESSAGE_QUEUE_H
#define MESSAGE_QUEUE_H

#include "message.h"
#include "queue.h"
#include <pthread.h>
#include <sys/types.h>

typedef struct MessageQueue {
    queue_t* msgs;
    pthread_mutex_t* mutex;     //
} message_queue_t;

message_queue_t * message_queue_create(uint32_t n_msgs, uint16_t flags);
void message_queue_add(message_queue_t* queue, const message_t* msg);
message_t* message_queue_remove(message_queue_t* queue);

#endif