#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define MAX_THREADS 4
#define MAX_QUEUE 10

typedef void task_func(void *arg);

// Task
typedef struct   {
    task_func* func;
    void* arg;
} task_t;

typedef struct  {
    pthread_t* threads;     // 
    size_t num_threads;
    
    // task queue
    task_t task_queue[MAX_QUEUE];
    int queue_size;
    int queue_front;
    int queue_rear;

    pthread_mutex_t mutex;  // 
    pthread_cond_t cond;    //

    int shutdown;      // 线程池关闭状态
} threadpool_t;
threadpool_t *threadpool_create();
void threadpool_destroy(threadpool_t* pool);
void threadpool_add_task(threadpool_t *pool, task_func* func, void* arg);