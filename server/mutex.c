#include "mutex.h"
#include <assert.h>

pthread_mutex_t * mutex_create()
{
    pthread_mutex_t* mutex =  (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
    assert(mutex);    
    int result = pthread_mutex_init(mutex, NULL);
    assert(result == 0);
    
    return mutex;
}
