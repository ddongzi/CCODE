#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include "task.h"
// mutex
pthread_mutex_t * mutex_create();
void mutex_destroy(pthread_mutex_t *mutex);


// cond
pthread_cond_t* cond_create();
void cond_destroy(pthread_cond_t* cond);


// thread
typedef void* thread_start_func (void*);
typedef enum {
    THREAD_IDLE,
    THREAD_BUSY,
    THREAD_EXIT,
} thread_status;

typedef enum {
    THREAD_ROLE_WORKER = 0x1000,
} thread_role_t;

typedef struct Thread {
    pthread_t id;
    thread_status status;
    thread_role_t role;
    thread_start_func* start_func;
    void* start_func_arg;
    

} thread_t;
thread_t* thread_create(thread_role_t role , thread_start_func *, void* arg);
