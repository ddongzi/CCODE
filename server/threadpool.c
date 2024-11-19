#include "threadpool.h"

threadpool_t *threadpool_create()
{
    threadpool_t *pool = (threadpool_t*)malloc(sizeof(threadpool_t));
    pool->threads = (pthread_t*)malloc(MAX_THREADS * sizeof(pthread_t));
    pool->queue_size = 0;
    pool->queue_front = 0;
    pool->queue_rear = 0;
    pool->shutdown = 0;

    pthread_mutex_init(&pool->mutex, NULL);
    pthread_cond_init(&pool->cond, NULL);

    return pool;
}

void threadpool_destroy(threadpool_t* pool)
{
    pthread_mutex_lock(&pool->mutex);
    pool->shutdown = 1;
    pthread_cond_broadcast(&pool->cond);
    pthread_mutex_unlock(&pool->mutex);

    for (size_t i = 0; i < MAX_THREADS; i++) {
        pthread_join(pool->threads[i], NULL);
    }

    free(pool->threads);
    free(pool);
}

void threadpool_add_task(threadpool_t *pool, task_func* func, void* arg)
{
    pthread_mutex_lock(&pool->mutex);
    if (pool->shutdown) {
        pthread_mutex_unlock(&pool->mutex);
    }

    if (pool->queue_size == MAX_QUEUE) {
        printf("Task queue is full, waiting...\n");
        pthread_mutex_unlock(&pool->mutex);
        return;
    }

    task_t new_task = {func, arg};
    pool->task_queue[pool->queue_rear] = new_task;
    pool->queue_rear = (pool->queue_rear + 1) % MAX_QUEUE;
    pool->queue_size++;

    pthread_cond_signal(&pool->cond);
    pthread_mutex_unlock(&pool->mutex);
}

void* thread_worker(void *arg)
{
    threadpool_t* pool = (threadpool_t*)arg;
    
    while (1) {
        pthread_mutex_lock(&pool->mutex);

        // 等待条件变量通知任务到达
        while (pool->queue_size == 0 && !pool->shutdown) {
            pthread_cond_wait(&pool->cond, &pool->mutex);
        }

        // 如果关闭线程池，退出
        if (pool->shutdown) {
            pthread_mutex_unlock(&pool->mutex);
            break;
        }

        // 获取任务
        task_t task = pool->task_queue[pool->queue_front];
        pool->queue_front = (pool->queue_front + 1) % MAX_QUEUE;
        pool->queue_size--;

        // 执行任务
        task.func(task.arg);

        pthread_mutex_unlock(&pool->mutex);

    }
    pthread_exit(NULL);
} 

// 任务
void sample_task(void* arg)
{
    int* num = (int*) arg;
    printf("Tid[%lu]\t Task %d is being executed\n", pthread_self(), *num);
}



