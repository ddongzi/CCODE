#include "message_queue.h"
#include <assert.h>
#include "pthread.h"

message_queue_t * message_queue_create(uint32_t n_msgs, uint16_t flags)
{
    message_t* msg;
    message_queue_t* queue;

    queue = (message_queue_t*)calloc(1, sizeof(message_queue_t));
    assert(queue);
    queue->msgs = queue_create(100);
    while (n_msgs--) {
        msg = message_create(flags);
        queue_add(queue->msgs, msg);
    }
    queue->mutex = mutex_create();

    return queue;
}


void message_queue_add(message_queue_t* queue, const message_t* msg)
{
    assert(queue);
    assert(queue->mutex);
    pthread_mutex_lock(queue->mutex);
    queue_add(queue->msgs, msg);
    pthread_mutex_unlock(queue->mutex);
}

message_t* message_queue_remove(message_queue_t* queue)
{
    assert(queue);
    message_t* msg;
    pthread_mutex_lock(queue->mutex);
    msg = queue_remove_head(queue->msgs);
    pthread_mutex_unlock(queue->mutex);
    return msg;
}
