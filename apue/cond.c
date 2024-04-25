//
// Created by dong on 4/25/24.
//
#include "apue.h"
#include "pthread.h"
struct msg{
    struct msg* next;
};
struct msg *queue;

pthread_cond_t qready = PTHREAD_COND_INITIALIZER;
pthread_mutex_t qlock = PTHREAD_MUTEX_INITIALIZER;

// 获取消息时需要队列上锁，
void process_msg()
{
    struct msg *tmp;
    for ( ; ; ) {
        pthread_mutex_lock(&qlock);
        // 等待条件可以操作队列
        pthread_cond_wait(&qready, &qlock);
        while (tmp != NULL) {
            tmp = queue;
            queue = tmp->next;
            pthread_mutex_unlock(&qlock);
            // 处理取出来的消息
        }
    }
}
void enqueue_msg(struct msg *mp)
{
    pthread_mutex_lock(&qlock);
    mp->next = queue;
    queue = mp;
    pthread_mutex_unlock(&qlock);
    pthread_cond_signal(&qready);
}