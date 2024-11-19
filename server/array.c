#include "array.h"
#include <stdlib.h>
#include <assert.h>

// public interfaces
/**
 * @brief 加入一个entry，返回其index
 * 
 * @param [in] arr 
 * @param [in] entry 
 * @return int 
 */
int array_add(array_t* arr, void* entry)
{
    size_t i;
    for (i = 0; i < arr->n_entries; i++) {
        if (!arr->entry_arr[i])
            break;
    }
    if (i >= arr->n_entries)
        return -1;
    arr->entry_arr[i] = entry;
    return i;
}
void array_clear(array_t* arr, size_t index)
{
    assert(arr);
    assert(index < arr->n_entries);    
    arr->entry_arr[index] = NULL;
}
array_t* array_create(size_t size)
{
    array_t* arr = (array_t *)calloc(1, sizeof(array_t));
    assert(arr);
    arr->entry_arr = (void**)calloc(size, sizeof(void*));
    assert(arr->entry_arr);
    arr->n_entries = size;
    return arr;
}
void*  array_get(array_t* arr, size_t index)
{
    assert(arr);
    assert(index < arr->n_entries);
    return arr->entry_arr[index];
}
/**
 * @brief 向一个空位置设定内容
 * 
 * @param [in] arr 
 * @param [in] entry 
 * @param [in] index 
 * @return void* 
 */
void*  array_set(array_t* arr, const void* entry, size_t index)
{
    assert(arr);
    assert(index < arr->n_entries);
    assert(!(arr->entry_arr[index]));
    arr->entry_arr[index] = entry;
    return arr->entry_arr[index];
}
size_t array_size(array_t* arr)
{
    assert(arr);
    return arr->n_entries;
}
index_t array_find(array_t* arr, array_find_func* func, void* entry)
{
    
    assert(arr);
    assert(func);
    for (size_t i = 0; i < arr->n_entries; i++) {
        if (arr->entry_arr[i] && func(arr->entry_arr[i], entry))
            return i;
    }
    return -1;
    
}