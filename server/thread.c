#include "thread.h"
#include <assert.h>

// ========= mutex ===========
// public interface
pthread_mutex_t * mutex_create()
{
    pthread_mutex_t* mutex =  (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
    assert(mutex);    
    int result = pthread_mutex_init(mutex, NULL);
    assert(result == 0);
    
    return mutex;
}
void mutex_destroy(pthread_mutex_t *mutex)
{
    free(mutex);
}
// ========= cond ===========
// public interface
pthread_cond_t* cond_create()
{
    pthread_cond_t* cond = (pthread_cond_t*)malloc(sizeof(pthread_cond_t));
    assert(cond);
    int result = pthread_cond_init(cond, NULL);
    assert(result == 0);
    return cond;
}
void cond_destroy(pthread_cond_t* cond)
{
    free(cond);
}



// ========= thread ===========
// public interface
/**
 * @brief 
 * 
 * @param [in] role 
 * @param [in] start_func 
 * @param [in] arg 
 * @return thread_t* 
 */
thread_t* thread_create(thread_role_t role, void* (*start_func)(void*), void* arg)
{
    assert(start_func);
    pthread_t thread_id;
    int result;
    thread_t* thread = (thread_t*)calloc(1, sizeof(thread_t));
    assert(thread);
    result = pthread_create(&thread_id, NULL, start_func, arg);
    assert(result == 0);
    thread->id = thread_id;
    thread->status = THREAD_IDLE;
    thread->start_func = start_func;
    return thread;
}
