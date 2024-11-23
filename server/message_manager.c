#include "message_manager.h"
#include "log.h"
#include "message_queue.h"
#include <assert.h>
#include "threadpool.h"

// define, enum
enum message_allocation_policy {
    MSG_N_PERM_MSG = 8, // 程序启动分配8个msg, 永久存在
    MSG_N_CONN_MSG = 2, // 每个连接分配两个message
};

// static property
// 当前消息统计
static uint32_t n_cur_msgs; 
static uint32_t n_cur_conn_msgs;   // 与connection有关的msgs ？？
static uint32_t n_cur_zombie_msgs;  //  与connection有关的,待销毁的
static uint32_t n_cur_disc_msgs;    // 与connection有关的,待丢弃的

static message_queue_t* free_msg_queue;
static message_queue_t* in_msg_queue;
static message_queue_t* out_msg_queue;

static uint32_t n_conn_msgs = MSG_N_CONN_MSG;   // 每次连接创建2个消息
static uint32_t n_perm_msgs = MSG_N_PERM_MSG;



// static functions
static bool check_msg_flags(void* msg, void* flags)
{
    return message_flags((message_t*)msg) & *(uint16_t*)flags;
}
static void task_recv_msg(void* arg)
{
    message_t* msg = (message_t*) arg;
    LOG_INFO("task recv msg callback, TID[%lu], %u, %s", pthread_self() ,msg->original_seqno, msg->body);
}


// public interfaces

void message_manager_init()
{
    LOG_INFO("Message manager init.");
    // 
    LOG_INFO("creating the free message queue with %d permanent messages", n_perm_msgs);
    free_msg_queue = message_queue_create(n_perm_msgs, MSG_LIFE_PERM);
    n_cur_msgs = n_perm_msgs;

    // create empty queue
    in_msg_queue = message_queue_create(0, 0);
    out_msg_queue = message_queue_create(0, 0);

    // TODO 信号量
}

/**
 * @brief 把当前的conn msgs放入队列
 * 
 */
void message_manager_create_conn_msgs()
{
    message_t* msg;
    uint32_t n_msgs = n_conn_msgs;
    LOG_INFO("creating %u connection messages", n_msgs);

    while(n_msgs--) {
        msg = message_create(MSG_LIFE_CONN);
        LOG_INFO("adding a connection message (%x) to the free message queue", msg->seqno);
        message_queue_add(free_msg_queue, msg);
        n_cur_msgs++;
        n_cur_conn_msgs++;
    }
    LOG_INFO("now %d connection messages, %d total messages", n_cur_conn_msgs, n_cur_msgs);
}
message_t * message_manager_get_free_msg()
{
    message_t* msg;
    msg = message_queue_remove(free_msg_queue);

    // there's a free message available
    assert(msg);
    return msg;

    // todo, msg 仓库不够
    
}
/**
 * @brief 
 * 
 * @param [in] msg 
 */
void message_manager_dispose_msg(message_t* msg)
{
    assert(msg);
    if (msg->flags & MSG_LIFE_DISC) {
        // destroys a message that have discardable lifetime
        LOG_INFO("destroying a discardable message");
        message_destroy(msg);
        assert(n_cur_disc_msgs);
        n_cur_disc_msgs--;
        n_cur_msgs--;
        return;
    }
    if (msg->flags & MSG_LIFE_CONN) {
        // destroys a connection zombie message
        // todo ???
        LOG_INFO("destroying a discardable message");
        message_destroy(msg);
        assert(n_cur_conn_msgs);
        n_cur_zombie_msgs--;
        if (!n_cur_zombie_msgs)
            LOG_INFO("no more zobie msg");
        n_cur_conn_msgs--;
        n_cur_msgs--;
        return;
    }
    // 退还到free msg 仓库
    message_queue_add(free_msg_queue, msg);

}
/**
 * @brief destroy msgs with the same flags(lifetime)
 * 
 * @param [in] msg 
 * @param [in] nmsgs 
 * @param [in] flags 
 * @return size_t 
 */
size_t message_manager_destroy_msgs(message_queue_t* queue, size_t nmsgs, uint16_t flags)
{
    message_t* msg;
    pthread_mutex_lock(queue->mutex);
    size_t ndestroyed = 0;
    for(;;) {
        if (!nmsgs--) {
            break;
        }
        LOG_INFO("");
        msg = (message_t*)queue_remove(queue, check_msg_flags, &flags);
        // 
        if (!msg) {
            LOG_WARN("can't destroy msg. queue remove failed.");
            break;
        }
        LOG_INFO("destroying message #%u", msg->seqno);
        message_destroy(msg);
        ndestroyed++;
    }
    pthread_mutex_unlock(queue->mutex);
    LOG_INFO("%d messages with flags 0x%04X were destroyed", ndestroyed, flags);
    return ndestroyed;
}

void message_manager_add_inmsg(const message_t* msg)
{
    void* arg = msg;    
    threadpool_add_task(task_recv_msg, msg, THREAD_ROLE_WORKER);
}