/**
 * @file queue.h
 * @author your name (you@domain.com)
 * @brief 优化的队列，会维持一个永久结点队列（仓库），优先从仓库中获取结点去构建队列，避免频繁释放和分配
 * @version 0.1
 * @date 2024-11-16
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#ifndef QUEUE_H
#define QUEUE_H

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct queue_node_t queue_node_t;
struct queue_node_t{
    void *entry;
    queue_node_t* next;
    uint8_t is_perm; //
};

typedef struct  {
    queue_node_t *head;
    queue_node_t *tail;
    size_t n_entries;
    size_t n_perm_nodes;
    queue_node_t* permhead;
    queue_node_t* permtail;
} queue_t;
typedef bool queue_find_func(void* entry1, void* entry2);

queue_t* queue_create(size_t n_entries);
void queue_destroy(queue_t* queue);
bool queue_isempty(const queue_t *queue);
size_t  queue_size(const queue_t* queue);
void queue_add(queue_t* queue, const void *entry);
void* queue_remove(queue_t* queue, queue_find_func* func, void* arg);
void* queue_remove_head(queue_t* queue);

#endif