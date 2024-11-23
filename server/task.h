#ifndef TASK_H
#define TASK_H
#include <stdint.h>

#define TASK_TYPE(task) ((task->id) & 0xf000)

typedef void task_func(void *arg);


// Task
typedef struct   {
    uint16_t id;    // 4bit threadtype| 

    task_func* func;
    void* arg;
} task_t;

#endif

