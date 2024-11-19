/**
 * @file array.h
 * @author your name (you@domain.com)
 * @brief 定长数组。非顺序排列，空桶插入。元素类型不固定，只是指针void*
 * @version 0.1
 * @date 2024-11-16
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#ifndef ARRAY_H
#define ARRAY_H

#include <sys/types.h>
#include <stdbool.h>

typedef size_t index_t;

typedef struct {
    void** entry_arr;  // 数组指针
    size_t n_entries;
} array_t;

typedef bool array_find_func(void* entry1, void* entry2);
int array_add(array_t* arr, void* entry);
void array_clear(array_t* arr, index_t index);
array_t* array_create(size_t size);
void*  array_get(array_t* arr, index_t index);
void*  array_set(array_t* arr, const void* entry, index_t index);
size_t array_size(array_t* arr);
index_t array_find(array_t* arr, array_find_func* func, void* entry);

#endif
