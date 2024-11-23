

#include "queue.h"
#include <assert.h>


// internal
/**
 * @brief 
 * 
 * @param [in] headp 
 * @param [in] tailp 
 * @param [in] node 
 */
static void add_tail(queue_node_t** headp, queue_node_t** tailp,queue_node_t *node)
{
    if (*tailp == NULL) {
                // 空queue
        *headp = *tailp = node;
    } else {
        (*tailp)->next = node;
        *tailp = node;
    }
    node->next = NULL;
}

/**
 * @brief 队列中安全拿走第一个node，
 * 
 * @param [in] headp 
 * @param [in] tailp 
 * @return queue_node_t* 
 */
static queue_node_t *remove_head(queue_node_t** headp, queue_node_t** tailp)
{
    queue_node_t *node = NULL;
    if (*headp) {
        node = *headp;
        *headp = (*headp)->next;
        if (!(*headp))
            *tailp = NULL;  //
    }
    return node;
}

/**
 * @brief 清理node，如果是永久结点，放回仓库； 如果不是，则free
 * 
 * @param [in] queue 
 * @param [in] node 
 */
static void dispose_node(queue_t* queue, queue_node_t* node)
{
    if(node->is_perm)
        add_tail(queue->permhead, queue->permtail, node);
    else
        free(node);
}

// public interfaces
/**
 * @brief 初始化一个队列。创建永久结点仓库
 * 
 * @param [in] n_entries 
 * @return queue_t* 
 */
queue_t* queue_create(size_t n_entries)
{
    queue_t* queue = (queue_t*)calloc(1, sizeof(queue_t));
    assert(queue);
    queue->n_perm_nodes = n_entries;
    while (n_entries--) {
        queue_node_t* node = (queue_node_t*)malloc(sizeof(queue_node_t));
        node->is_perm = 1;
        add_tail(&queue->permhead, &queue->permtail, node);
    }
    return queue;
}
void queue_destroy(queue_t* queue)
{
    assert(queue);
    assert(queue->n_entries == 0);    // 释放queue时候，保证queue内没有元素
    while (queue->n_perm_nodes --) {
        free(remove_head(&queue->permhead, &queue->permtail));
    }
    free(queue);
}
bool queue_isempty(const queue_t *queue)
{
    assert(queue);
    return !queue->n_entries;
}
size_t  queue_size(const queue_t* queue)
{
    return queue->n_entries;
}
/**
 * @brief 
 * 
 * @param [in] queue 
 * @param [in] entry : 实体指针
 */
void queue_add(queue_t* queue, const void *entry)
{
    queue_node_t* newnode;
    assert(queue);

    // 从永久结点（空闲）队列拿一个空闲的
    if (!(newnode = remove_head(&queue->permhead, &queue->permtail))) {
        newnode = (queue_node_t*)calloc(1, sizeof(queue_node_t));
        assert(newnode);
    }
    newnode->entry = entry;
    add_tail(&queue->head, &queue->tail, newnode);
    queue->n_entries++;
}

/**
 * @brief 根据find函数移除一个node
 * 
 * @param [in] queue 
 * @param [in] find_func : 比较entry
 * @param [in] arg 
 * @return void* ：entry ， NUll
 */
void* queue_remove(queue_t* queue, queue_find_func* find_func, void* arg)
{
    void* entry = NULL;
    queue_node_t* cur, *prev;
    cur = queue->head;
    prev = NULL;
    while(cur) {
        if (!find_func(cur->entry, arg)) {
            prev = cur;
            cur = cur->next;
            continue;
        }
        // 找到cur位置删除，
        if (prev)
            prev->next = cur->next;
        else    // 第一个位置删除
            queue->head = cur->next;
        
        if (!cur->next) // 最后一个位置删除
            queue->tail = prev;
        
        entry = cur->entry;
        dispose_node(queue, cur);
        queue->n_entries--;
        break;
    }
    return entry;
}
/**
 * @brief remove head
 * 
 * @param [in] queue 
 * @return void* ：first entry
 */
void* queue_remove_head(queue_t* queue)
{
    void* entry = NULL;
    queue_node_t* node;
    
    if (!(node = remove_head(&queue->head, &queue->tail))) {
        // 队列空
        return NULL;
    }
    entry = node->entry;
    dispose_node(queue, node);
    queue->n_entries--;
    return entry;
}
/**
 * @brief 获取第一个元素
 * 
 * @param [in] queue 
 * @return void* 
 */
void* queue_head(queue_t* queue)
{
    assert(queue);
    return queue->head->entry;
}
