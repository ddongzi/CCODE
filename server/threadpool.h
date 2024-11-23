#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include "thread.h"
#include <stdint.h>
#include "array.h"
#include "queue.h"
#include "task.h"

#define MAX_THREADS 4
#define MAX_QUEUE 10

typedef array_t thread_table;
typedef queue_t task_queue;

typedef struct ThreadPool {
    // task threads
    thread_table* worker_threads;     // 
    size_t num_worker_threads;
    // task queue
    task_queue *msg_queue;
    pthread_mutex_t* msg_queue_mutex;  // 
    pthread_cond_t* msg_queue_cond_not_empty;    //
    pthread_cond_t* msg_queue_cond_empty;    //


    // direct thread 
    thread_t* heartbeat_thread;

    int shutdown;      // 线程池关闭状态
} threadpool_t;
void threadpool_init();
void threadpool_run();

threadpool_t *threadpool_create();
void threadpool_destroy();
void threadpool_add_task(task_func* func, void* arg, thread_role_t type);

void threadpool_register(thread_start_func* func);