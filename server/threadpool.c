#include "threadpool.h"
#include "log.h"
#include <assert.h>

// static property
static threadpool_t* pool;
static uint16_t last_taskid = 0x0000;

// static functions
static void* thread_worker(void *arg);


static void add_queue_task(task_t* task)
{   
    pthread_mutex_lock(pool->msg_queue_mutex);
    queue_add(pool->msg_queue , task); 
    pthread_cond_signal(pool->msg_queue_cond_not_empty);  
    pthread_mutex_unlock(pool->msg_queue_mutex);
}
static void add_direct_task(task_t* task)
{
    LOG_INFO("add direct task . TODO");
}

/**
 * @brief 跟新last task id , return 
 * 
 * @param [in] type 
 * @return uint16_t 
 */
static uint16_t threadpool_get_last_taskid(thread_role_t type)
{
    uint16_t result;
    uint16_t high_bits = type & 0xf000;  // 提取并保留 type 的高 4 位
    uint16_t low_bits = last_taskid & 0x0fff;   // 提取并保留 last_taskid 的低 12 位

    // 递增低 12 位
    low_bits = (low_bits + 1) & 0x0fff;  // 确保低 12 位不会溢出

    // 将 type 的高 4 位和更新后的低 12 位合并
    result = high_bits | low_bits;  // 保持 high_bits 来自 type，low_bits 递增

    last_taskid = result;  // 更新 last_taskid
    return result;
}

static worker_thread_init()
{
    thread_t* thread;

    pool->msg_queue_mutex = mutex_create();
    pool->msg_queue_cond_not_empty = cond_create();
    pool->msg_queue_cond_empty = cond_create();
    pool->msg_queue = queue_create(MAX_QUEUE);

    pool->worker_threads = array_create(MAX_THREADS);
    for (size_t i = 0; i < MAX_THREADS; i++) {
        thread =  thread_create(THREAD_ROLE_WORKER, thread_worker, pool);
        LOG_INFO("add #%d worker thread. thread id #%lu", i, thread->id);
        array_add(pool->worker_threads, thread);
        pool->num_worker_threads += 1;
    }

}
static worker_thread_destroy()
{
    thread_t *thread;
    int result;
    // 只有队列空才能销毁
    pthread_mutex_lock(pool->msg_queue_mutex);
    while (!queue_isempty(pool->msg_queue)) {
        pthread_cond_wait(pool->msg_queue_cond_empty, pool->msg_queue_mutex);
    }
    pthread_mutex_unlock(pool->msg_queue_mutex);

    for (size_t i = 0; i < MAX_THREADS; i++) {
        thread = array_get(pool->worker_threads, i);
        result = pthread_join(thread->id, NULL);
        assert(result == 0);
        free(array_get(pool->worker_threads, i));
        LOG_INFO("destroy thread# %d.", i);
    }
    queue_destroy(pool->msg_queue);
    mutex_destroy(pool->msg_queue_mutex);
    cond_destroy(pool->msg_queue_cond_empty);
    cond_destroy(pool->msg_queue_cond_not_empty);
    free(pool->worker_threads);
}
/**
 * @brief 工作线程， 消费队列task
 * 
 * @param [in] arg : &&pool
 * @return void* 
 */
static void* thread_worker(void *arg)
{
    LOG_INFO("thread worker start");
    threadpool_t* pool = (threadpool_t*)arg;

    while (1) {
        pthread_mutex_lock(pool->msg_queue_mutex);

        // 等待条件变量通知任务到达
        while (queue_isempty(pool->msg_queue) && !pool->shutdown) {
            pthread_cond_wait(pool->msg_queue_cond_not_empty, pool->msg_queue_mutex);
        }
        // 如果关闭线程池，退出
        if (pool->shutdown) {
            pthread_mutex_unlock(pool->msg_queue_mutex);
            break;
        }
        task_t* task = (task_t*)queue_remove_head(pool->msg_queue);

        if (queue_isempty(pool->msg_queue)) {
            pthread_cond_signal(pool->msg_queue_cond_empty);
        }

        // 执行任务
        task->func(task->arg);

        pthread_mutex_unlock(pool->msg_queue_mutex);

    }
    pthread_exit(NULL);
    return (void*)22;
} 


// public interfaces
void threadpool_init()
{
    pool = (threadpool_t*)malloc(sizeof(threadpool_t));
    worker_thread_init();
    pool->shutdown = 1;
}
void threadpool_run()
{
    pool->shutdown = 0;
}
/**
 * @brief 外部注册线程 TODO
 * 
 * @param [in] func 
 */
void threadpool_register(thread_start_func* func)
{
    // thread_t* thread = thread_create(func);
}


void threadpool_destroy()
{
    // worker thread
    pthread_mutex_lock(pool->msg_queue_mutex);
    pool->shutdown = 1;
    pthread_cond_broadcast(pool->msg_queue_cond_not_empty);
    pthread_mutex_unlock(pool->msg_queue_mutex);

    worker_thread_destroy();

    free(pool);
}

/**
 * @brief 生产者，添加task
 * 
 * @param [in] func :void f(void *arg) 任务函数
 * @param [in] arg : arg 函数参数
 * @param [in] type ：设置对应thread role
 */
void threadpool_add_task(task_func* func, void* arg, thread_role_t type)
{

    task_t* task = (task_t*)malloc(sizeof(task_t));
    assert(task);
    task->id = threadpool_get_last_taskid(type);
    task->func = func;
    task->arg = arg;
    if (TASK_TYPE(task) == THREAD_ROLE_WORKER) {
        add_queue_task(task);
    } else {
        add_direct_task(task);
    }


}



