//
// Created by dong on 4/25/24.
//
#include <fcntl.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <assert.h>

/* 与getline交互*/
typedef struct {
    char *buffer;
    size_t buffer_length;
    size_t input_length;
} input_buffer_t;

/*源命令 结果*/
typedef enum {
    META_COMMAND_SUCCESS, // 源命令识别成功
    META_COMMAND_UNRECOGNIZED_COMMAND // 。开始， 但是命令不存在
} meta_cmd_res;

/*sql命令 结果*/
typedef enum {
    PREPARE_SUCCESS,
    PREPARE_UNRECOGNIZED_STATEMENT,
    PREPARE_SYNTAX_ERROR,
    PREPARE_STRING_TOO_LONG,
    PREPARE_NEGATIVE_ID
} prepare_res_type;

/*sql 命令类型*/
typedef enum {
    STATEMENT_INSERT,
    STATEMENT_SELECT,
    STATEMENT_DELETE
} statement_type;

#define COLUMN_USERNAME_SIZE 32
#define COLUMN_EMAIL_SIZE 255
typedef struct {
    uint32_t id;
    char user_name[COLUMN_USERNAME_SIZE + 1];
    char email[COLUMN_EMAIL_SIZE + 1];
} row_t;

typedef struct {
    statement_type type;
    row_t row;
} sql_statement;

// 命令执行结果
typedef enum {
    EXECUTE_SUCCESS,
    EXECUTE_TABLE_FULL,
    EXECUTE_DUPLICATE_KEY
} execute_res;

#define size_of_attribute(type, attr) sizeof(((type*)0)->attr)

const uint32_t ID_SIZE = size_of_attribute(row_t, id);
const uint32_t USER_NAME_SIZE = size_of_attribute(row_t, user_name);
const uint32_t EMAIL_SIZE = size_of_attribute(row_t, email);
const uint32_t ID_OFFSET = 0;
const uint32_t USER_NAME_OFFSET = ID_OFFSET + ID_SIZE;
const uint32_t EMAIL_OFFSET = USER_NAME_OFFSET + USER_NAME_SIZE;
const uint32_t ROW_SIZE = ID_SIZE + USER_NAME_SIZE + EMAIL_SIZE;

void serialize_row(row_t* source, void* destination);

#define BTREE_M 5 // TODO 暂时没有用，因为叶子节点没有限制
const uint32_t PAGE_SIZE = 4096; // 每个页 4096 个字节
#define TABLE_MAX_PAGES 100
#define INVALID_PAGE_NUM UINT32_MAX  // internal node为空节点 : right child page_num 为 INVALID_PAGE_NUM

/*使用 pager 访问页缓存和文件*/
typedef struct {
    int fd;
    uint32_t file_length; // 追加写。 写的起始位置. 是page_size 整数倍
    uint32_t num_pages; // 页个数
    void *pages[TABLE_MAX_PAGES];
} pager_t;

/* 单表 : page数组*/
typedef struct {
    uint32_t root_page_num; // B-tree 根节点所在页
    pager_t* pager;
} table_t;

/* 代表在table中的位置， #pagenum#cellnum
 * 1. 指向表起始和表尾部
 * 2. 通过cursor 进行insert、select、delete、update， search for a ID , then cursor pointing this ID row。
 * */
typedef struct {
    table_t *table;
    uint32_t page_num; //
    uint32_t cell_num;
    bool end_of_table; // 帮助insert
} cursor_t;

/* 对应关系： 一个节点对应一个页。 根节点头部存储子节点page_num来定位。*/

/* B-tree 内部节点存储指针和部分key， 叶子节点存储key和val */
typedef enum {NODE_INTERNAL, NODE_LEAF} node_type;


/* ============ Common header layout ============*/
/*
 * | Node Type (uint8_t)      | 1 byte    | 
 * | Is Root (uint8_t)        | 1 byte    | 
 * | Parent Pointer (uint32_t)| 4 bytes   | root节点parent是Invalid_page_num
 * / Page num                 | 4 bytes   |
 * |--------------------------------------|
 * | Total Header Size        | 6 bytes   |
 * =========================================
 */
const uint8_t NODE_TYPE_SIZE = sizeof(uint8_t);
const uint32_t NODE_TYPE_OFFSET = 0;
const uint8_t IS_ROOT_SIZE = sizeof(uint8_t);
const uint32_t IS_ROOT_OFFSET = NODE_TYPE_SIZE;
const uint32_t PARENT_POINTER_SIZE = sizeof(uint32_t); // pagenum
const uint32_t PARENT_POINTER_OFFSET = IS_ROOT_OFFSET + IS_ROOT_SIZE;
const uint32_t PAGE_NUM_SIZE = sizeof(uint8_t);
const uint32_t PAGE_NUM_OFFSET = PARENT_POINTER_OFFSET + PARENT_POINTER_SIZE;
const uint32_t COMMON_NODE_HEADER_SIZE = NODE_TYPE_SIZE + IS_ROOT_SIZE + PARENT_POINTER_SIZE;

/* ============ Leaf Node Structure ============ */
/* 
 * ============ Leaf Node Header Layout ============ 
 * | Number of Cells                  | 4 bytes    | 设置有效cell
 * | Next Leaf Pointer                | 4 bytes    | 
 * ============ Leaf Node Body Layout #0 cell ====== 
 * | Key                              | 4 bytes    | 
 * | Value (ROW_SIZE)                 | ROW_SIZE   |
 * ============ Leaf Node Body Layout #1 cell ====== 
 * | Key                              | 4 bytes    | 
 * | Value (ROW_SIZE)                 | ROW_SIZE   |
 * |     .....................................     |
 * ================================================
 * | Space for Cells                  | PAGE_SIZE - 14 bytes |
 */

/* leaf node header layout*/
const uint32_t LEAF_NODE_NUM_CELLS_SIZE = sizeof(uint32_t);
const uint32_t LEAF_NODE_NUM_CELLS_OFFSET = COMMON_NODE_HEADER_SIZE;

const uint32_t LEAF_NODE_NEXT_LEAF_SIZE = sizeof(uint32_t);
// 最右侧叶子节点 的next leaf为0， 表示没有下一个兄弟，0为保守的root页
const uint32_t LEAF_NODE_NEXT_LEAF_OFFSET = LEAF_NODE_NUM_CELLS_OFFSET + LEAF_NODE_NUM_CELLS_SIZE ; // 指向下一个页

const uint32_t LEAF_NODE_HEADER_SIZE = COMMON_NODE_HEADER_SIZE + LEAF_NODE_NUM_CELLS_SIZE + LEAF_NODE_NEXT_LEAF_SIZE;

/* leaf node body layout  */
const uint32_t LEAF_NODE_KEY_SIZE = sizeof(uint32_t);
const uint32_t LEAF_NODE_KEY_OFFSET = 0; // cell 内偏移
const uint32_t LEAF_NODE_VALUE_SIZE = ROW_SIZE;
const uint32_t LEAF_NODE_VALUE_OFFSET = LEAF_NODE_KEY_OFFSET + LEAF_NODE_KEY_SIZE; // cell内偏移
const uint32_t LEAF_NODE_CELL_SIZE = LEAF_NODE_KEY_SIZE + LEAF_NODE_VALUE_SIZE;
const uint32_t LEAF_NODE_SPACE_FOR_CELLS = PAGE_SIZE - LEAF_NODE_HEADER_SIZE;
#if defined(DEBUG)
    const uint32_t LEAF_NODE_MAX_CELLS = 3;
#else
    const uint32_t LEAF_NODE_MAX_CELLS = LEAF_NODE_SPACE_FOR_CELLS / LEAF_NODE_CELL_SIZE;
#endif

/* 满节点均分*/
const uint32_t LEAF_NODE_RIGHT_SPLIT_COUNT = (LEAF_NODE_MAX_CELLS + 1) / 2;
const uint32_t LEAF_NODE_LEFT_SPLIT_COUNT = (LEAF_NODE_MAX_CELLS + 1) - LEAF_NODE_RIGHT_SPLIT_COUNT;
const uint32_t LEAF_NODE_MIN_CELLS = LEAF_NODE_RIGHT_SPLIT_COUNT; // 最小cell数

uint32_t *leaf_node_num_cells(void *node);
uint32_t *leaf_node_key(void *node, uint32_t cell_num);
void *leaf_node_cell(void *node, uint32_t cell_num);
void *leaf_node_value(void *node, uint32_t cell_num);
cursor_t *leaf_node_find(table_t *table, uint32_t page_num, uint32_t key);
uint32_t *leaf_node_next_leaf(void *node);

void *leaf_node_left_bro(void *node);
void *leaf_node_right_bro(void *node);


/* ============ Internal Node Structure ============ */
/* 
 *  ============ Internal Node Header Layout  ====== 
 * | Number of Keys (uint32_t)        | 4 bytes    | 
 * | Right Child Pointer (uint32_t)    | 4 bytes    | 
 *  ============ Leaf Node Body Layout #0 cell ====== 
 * | Child Pointer (uint32_t)        | 4 bytes    | 
 * | Key (uint32_t)                  | 4 bytes    | 孩子节点树key最值
 *  ============ Leaf Node Body Layout #1 cell ====== 
 * | Child Pointer (uint32_t)        | 4 bytes    | 
 * | Key (uint32_t)                  | 4 bytes    | 
 */

/* Internal Node header layout */

const uint32_t INTERNAL_NODE_NUM_KEYS_SIZE = sizeof(uint32_t); // TODO ?
const uint32_t INTERNAL_NODE_NUM_KEYS_OFFSET = COMMON_NODE_HEADER_SIZE;
const uint32_t INTERNAL_NODE_RIGHT_CHILD_SIZE = sizeof(uint32_t);
const uint32_t INTERNAL_NODE_RIGHT_CHILD_OFFSET = INTERNAL_NODE_NUM_KEYS_OFFSET + // 最右侧孩子的page_num
        INTERNAL_NODE_NUM_KEYS_SIZE;
const uint32_t INTERNAL_NODE_HEADER_SIZE = COMMON_NODE_HEADER_SIZE +
        INTERNAL_NODE_NUM_KEYS_SIZE + INTERNAL_NODE_RIGHT_CHILD_SIZE;

/* Internal Node body layout
 * 每个cell 包含孩子指针和 key
 */
const uint32_t INTERNAL_NODE_KEY_SIZE = sizeof(uint32_t); // 孩子key的最值
const uint32_t INTERNAL_NODE_CHILD_SIZE = sizeof(uint32_t);
const uint32_t INTERNAL_NODE_CELL_SIZE = INTERNAL_NODE_KEY_SIZE + INTERNAL_NODE_CHILD_SIZE;
#ifdef DEBUG
    const uint32_t INTERNAL_NODE_MAX_CELLS = 4;
#else
    const uint32_t INTERNAL_NODE_MAX_CELLS = 5;
#endif

void create_new_root(table_t *table, uint32_t right_child_page_num);
uint32_t *internal_node_num_keys(void *node);
uint32_t* internal_node_right_child(void* node);
uint32_t* internal_node_cell(void* node, uint32_t cell_num);
uint32_t* internal_node_child(void* node, uint32_t cell_num);
uint32_t* internal_node_key(void *node, uint32_t cell_num);
cursor_t* internal_node_find(table_t *table, uint32_t page_num, uint32_t key);
void internal_node_split_and_insert(table_t * table, uint32_t parent_page_num, uint32_t child_page_num);
void initialize_internal_node(void *root);
void update_internal_node_key(void *node, uint32_t old_key, uint32_t new_key);
void internal_node_insert(table_t * table, uint32_t parent_page_num, uint32_t child_page_num);
uint32_t internal_node_find_child(void* node, uint32_t key);
uint32_t* node_parent(void* node);

/* 在表中根据key找到 内部节点或者叶子节点， cursor*/
cursor_t *table_find(table_t *table, uint32_t key);

void *get_page(pager_t* pager, uint32_t page_num);
uint32_t get_unused_page_num(pager_t *pager);

void print_constants() ;
void indent(uint32_t level);  // 递归调用表现tab
void print_tree(pager_t * pager, uint32_t page_num, uint32_t indentation_level); // 打印tree
void print_table(table_t *table);

void leaf_node_split_and_insert(cursor_t *cursor, uint32_t key, row_t *val);

uint32_t* node_parent(void* node)
{
    return node + PARENT_POINTER_OFFSET;
}
node_type get_node_type(void *node)
{
    uint8_t type = * ((uint8_t *) (node + NODE_TYPE_OFFSET));
    return type;
}
void set_node_type(void *node, node_type type)
{
     *((uint8_t *) (node + NODE_TYPE_OFFSET)) = type;
}
/* 标识是否*/
bool is_node_root(void *node)
{
    uint8_t value = *((uint8_t *)(node + IS_ROOT_OFFSET));
    return (bool)value;
}
void set_node_root(void *node, bool is_root)
{
    uint8_t value = is_root;
    *((uint8_t *)(node + IS_ROOT_OFFSET)) = value;
}

/**
 * @brief 返回节点（树）的最大key
 *      如果是叶子节点：获取最右边元素的key
 *      如果是内部节点：递归向右获取
 * @param pager 
 * @param node 
 * @return uint32_t 
 */
uint32_t get_node_max_key(pager_t *pager, void *node)
{
    if (get_node_type(node) == NODE_LEAF) {
        return *(uint32_t *)leaf_node_key(node, *leaf_node_num_cells(node) - 1);
    }
    void* right_child = get_page(pager, *internal_node_right_child(node));
    return get_node_max_key(pager, right_child);
}

/*internal node */
uint32_t *internal_node_num_keys(void *node)
{
    return node + INTERNAL_NODE_NUM_KEYS_OFFSET;
}
uint32_t* internal_node_right_child(void* node)
{
    return node + INTERNAL_NODE_RIGHT_CHILD_OFFSET;
}
/**
 * @brief 获取内部节点#cell_num的child_pagenum
 * 
 * @param node 
 * @param cell_num 
 * @return uint32_t* 
 * @warning 不直接使用，可能越界。 通过internal_node_child获取
 */
uint32_t* internal_node_cell(void* node, uint32_t cell_num)
{
    return node + INTERNAL_NODE_HEADER_SIZE + cell_num * INTERNAL_NODE_CELL_SIZE;
}
/**
 * @brief 获取内部节点#cell_num的pagenum
 * 
 * @param node 
 * @param cell_num cell_num=num_keys 时候，表示获取最右孩子
 * @return uint32_t* child_node
 */
uint32_t* internal_node_child(void* node, uint32_t cell_num)
{
    uint32_t num_keys = *internal_node_num_keys(node);
    if (cell_num > num_keys) {
        printf("Tried to access child_num %d> num_keys %d\n", cell_num, num_keys);
        exit(EXIT_FAILURE);
    } else if (cell_num == num_keys) {
        // TODO
        uint32_t *right_child = internal_node_right_child(node);
        if (*right_child == INVALID_PAGE_NUM) {
            printf("Tried to access right child of node , but was invalid page.\n");
            exit(EXIT_FAILURE);
        }
        return right_child;
    } else {
        uint32_t *child = internal_node_cell(node, cell_num);
        if (*child  == INVALID_PAGE_NUM) {
            printf("Tried to access child %d of node , but was invalid page.\n", cell_num);
            exit(EXIT_FAILURE);
        }
        return child ;
    }
}

uint32_t* internal_node_key(void *node, uint32_t cell_num)
{
    return (void *)internal_node_cell(node, cell_num) + INTERNAL_NODE_CHILD_SIZE;
}
// uint32_t* internal_node_child

/* 在内部节点中找到包含这个key 的孩子索引*/
/**
 * @brief 
 * 
 * @param node 
 * @param key 
 * @return uint32_t cell_num 0 ~ numkeys-1
 * @warning 不涉及最右孩子pointer
 */
uint32_t internal_node_find_child(void* node, uint32_t key)
{
    uint32_t num_keys = *internal_node_num_keys(node);

    uint32_t low = 0;
    uint32_t high = num_keys;
    uint32_t mid, mid_key;
    while (low < high) {
        mid = (low + high) / 2;
        mid_key = *(uint32_t *)internal_node_key(node, mid);
        if (mid_key >= key) {
            high = mid;
        } else {
            low = mid + 1;
        }
    }
    return low;
}
/**
 * @brief 递归找到key所在的#pagenum#cellnum
 * 
 * @param table 
 * @param page_num 
 * @param key 
 * @return cursor_t* 
 * @warning 
 */
cursor_t* internal_node_find(table_t *table, uint32_t page_num, uint32_t key)
{
    void *node = get_page(table->pager, page_num);
    uint32_t num_keys = *internal_node_num_keys(node);

    uint32_t child_index = internal_node_find_child(node, key);
    uint32_t child_page_num = *internal_node_child(node, child_index);
    void *child = get_page(table->pager, child_page_num);
    switch (get_node_type(child)) {
        case NODE_LEAF:
            return leaf_node_find(table, child_page_num, key);
        case NODE_INTERNAL:
            return internal_node_find(table, child_page_num, key);
    }
}
/* 分裂内部节点插入
 * 1. 创建一个父亲节点
 * 2. parent_page_num ： 需要分割的内部节点。  child_page_num ; 新的叶子节点。 需要内部指向
 * 3.
 * */
void internal_node_split_and_insert(table_t * table, uint32_t parent_page_num, uint32_t child_page_num)
{
    uint32_t old_page_num = parent_page_num;
    void * old_node = get_page(table->pager, old_page_num);
    uint32_t old_max = get_node_max_key(table->pager, old_node);

    void* child = get_page(table->pager, child_page_num);
    uint32_t child_max = get_node_max_key(table->pager, child);

    uint32_t new_page_num = get_unused_page_num(table->pager);

    uint32_t splitting_root = is_node_root(old_node); // 记录被分裂节点是否是根节点， 如果是根节点，还需要创建root
    void* parent;
    void * new_node;
    if (splitting_root) {
        create_new_root(table, new_page_num);
        parent = get_page(table->pager, table->root_page_num);
        old_page_num = *internal_node_child(parent, 0); // 将左子节点设置为parent 的第一个cell
        old_node = get_page(table->pager, old_page_num);
    } else {
        // 如果不是根节点分裂, 只需要链接新页
        parent = get_page(table->pager, *node_parent(old_node));
        new_node = get_page(table->pager, new_page_num);
        initialize_internal_node(new_node);
    }

    // 1. 先将最右孩子给 new_page
    uint32_t * old_num_keys = internal_node_num_keys(old_node);
    uint32_t cur_page_num = *internal_node_right_child(old_node); // 先指向最右侧孩子
    void* cur = get_page(table->pager, cur_page_num);

    internal_node_insert(table, new_page_num, cur_page_num); //
    *node_parent(cur) = new_page_num;
    *internal_node_right_child(old_node) = INVALID_PAGE_NUM; // 原来page 最右侧孩子指向空

    // 2. 把中间靠右的挪到new_poge
    for (int i = INTERNAL_NODE_MAX_CELLS - 1; i > INTERNAL_NODE_MAX_CELLS / 2; --i) {
        cur_page_num = *internal_node_child(old_node, i);
        cur = get_page(table->pager, cur_page_num);
        internal_node_insert(table, new_page_num, cur_page_num);
        *node_parent(cur) = new_page_num;

        (*old_num_keys)--;
    }

    // 3. 设置原node 的右边孩子
    *internal_node_right_child(old_node) = *internal_node_child(old_node, *old_num_keys - 1);
    (*old_num_keys)--;

    // 4. 新插入节点进行关联
    uint32_t max_after_split = get_node_max_key(table->pager, old_node);
    uint32_t destination_page_num = child_max < max_after_split ? old_page_num : new_page_num;

    internal_node_insert(table, destination_page_num, child_page_num);
    *node_parent(child) = destination_page_num;

    // 5. 收尾
    update_internal_node_key(parent, old_max, get_node_max_key(table->pager, old_node)); // 更新父节点

    if (!splitting_root) {
        internal_node_insert(table, *node_parent(old_node), new_page_num);
        *node_parent(new_node) = *node_parent(old_node);
    }

}


void update_internal_node_key(void *node, uint32_t old_key, uint32_t new_key)
{
    // 获取旧key所在的孩子索引 TODO
    uint32_t old_child_index = internal_node_find_child(node, old_key);
    *internal_node_key(node, old_child_index) = new_key;
}

/**
 * @brief 内部节点插入一个cell，
 * 
 * @param table 
 * @param parent_page_num :该parent插入
 * @param child_page_num :chiLd_pointer cell值
 */
void internal_node_insert(table_t * table, uint32_t parent_page_num, uint32_t child_page_num)
{
    void *parent = get_page(table->pager, parent_page_num);
    void *child = get_page(table->pager, child_page_num);
    uint32_t child_max_key = get_node_max_key(table->pager, child);
    uint32_t index = internal_node_find_child(parent,child_max_key); // 要插入的位置，插入max_key

    uint32_t original_num_keys = *internal_node_num_keys(parent);
    if (original_num_keys >= INTERNAL_NODE_MAX_CELLS) {
        // parent节点容量超了， 需要分割 parent节点
        internal_node_split_and_insert(table, parent_page_num, child_page_num);
        return;
    }

    uint32_t right_child_page_num = *internal_node_right_child(parent);

    if (right_child_page_num == INVALID_PAGE_NUM) {
        // parent为空节点 : 设置right child 即可
        *internal_node_right_child(parent) = child_page_num;
        return;
    }

    void *right_child = get_page(table->pager, right_child_page_num);
    *internal_node_num_keys(parent) = original_num_keys + 1;

    if (child_max_key > get_node_max_key(table->pager, right_child)) {
        // 如果大于最右侧孩子， 代替最右孩子字段， 将最右孩子写到内部节点最后中
        *internal_node_child(parent, original_num_keys) = right_child_page_num;
        *internal_node_key(parent, original_num_keys) = get_node_max_key(table->pager, right_child);
        *internal_node_right_child(parent) = child_page_num;
    } else {
        // 从index后移
        for (uint32_t i = original_num_keys; i > index; --i) {
            void *destination = internal_node_cell(parent, i);
            void *source = internal_node_cell(parent, i - 1);
            memcpy(destination, source, INTERNAL_NODE_CELL_SIZE);
        }
        *internal_node_child(parent, index) = child_page_num;
        *internal_node_key(parent, index) = child_max_key;
    }
}

/**
 * @brief 移除节点
 * 
 * @param cursor 
 * @param key 
 */
void internal_node_remove(cursor_t *cursor, uint32_t key)
{

}

/**
 * @brief 对一个node初始化为内部节点，
 * 
 * @param node 
 */
void initialize_internal_node(void *node)
{
    set_node_type(node, NODE_INTERNAL);
    set_node_root(node, false);

    *internal_node_num_keys(node) = 0;
    *internal_node_right_child(node) = INVALID_PAGE_NUM;
}

/*=================leaf node ================================*/
// 如果内部节点获取非叶子节点专属字段， 预期地址值应该为0？
/* 返回 num_cells 对应地址*/
/**
 * @brief 
 * 
 * @param node 
 * @return uint32_t* 
 */
uint32_t *leaf_node_num_cells(void *node)
{
    return node + LEAF_NODE_NUM_CELLS_OFFSET;
}
/**
 * @brief  返回cell_num的 cell
 * 
 * @param node 
 * @param cell_num 
 * @return void* :
 */
void *leaf_node_cell(void *node, uint32_t cell_num)
{
    return node + LEAF_NODE_HEADER_SIZE + cell_num * LEAF_NODE_CELL_SIZE;
}
/**
 * @brief 获取node#cell_num 的key
 * 
 * @param node 
 * @param cell_num 
 * @return uint32_t* : 指向key
 */
uint32_t *leaf_node_key(void *node, uint32_t cell_num)
{
    return leaf_node_cell(node, cell_num);
}
/* 返回 cell VALUE的地址*/
void *leaf_node_value(void *node, uint32_t cell_num)
{
    return leaf_node_cell(node, cell_num) + LEAF_NODE_VALUE_OFFSET;
}
/**
 * @brief 返回右边兄弟节点（共同父亲）指针，
 * @param node 
 * @return uint32_t* ：右边兄弟节点地址；如果node是最右孩子，next_leaf指向 #page_num0节点(root节点)
 * @warning 
 */
uint32_t *leaf_node_next_leaf(void *node)
{
    return node + LEAF_NODE_NEXT_LEAF_OFFSET;
}

/**
 * @brief 返回左边兄弟节点指针，
 * 
 * @param node 
 * @return uint32_t* 
 */
uint32_t *leaf_node_prev_leaf(void *node)
{
    
}

/**
 * @brief 返回leaf_node对应在父亲节点内的cell_num
 * 
 * @return uint32_t ：返回leaf_node对应在父亲节点内的cell-key
 * @warning ！！！ TODO如果是最右侧孩子， 还没涉及实现
 */
uint32_t leaf_node_cell_num(void *node)
{
    uint32_t parent_page_num = *node_parent(node);
    void *parent = get_page(cursor->table->pager, parent_page_num);
    
    for (size_t i = 0; i < internal_node_num_keys(parent); i++) {

        // 节点需要存储page_num        
        // if (*internal_node_child(node, i) == ) {
            
        // }
    }
    
}

/* 初始化叶子节点， cell_num 为0*/
void initialize_leaf_node(void *node)
{
    set_node_type(node, NODE_LEAF);
    set_node_root(node, false);
    *leaf_node_num_cells(node) = 0;
    *leaf_node_next_leaf(node) = 0;
}
/* 根据key ，返回cursor*/
cursor_t *leaf_node_find(table_t *table, uint32_t page_num, uint32_t key)
{
    void *node = get_page(table->pager, page_num);
    uint32_t num_cells = *((uint32_t *)leaf_node_num_cells(node));

    cursor_t *cursor = (cursor_t *) malloc(sizeof(cursor_t));
    cursor->table = table;
    cursor->page_num = page_num;

    // 二分搜索
    uint32_t low = 0;
    uint32_t high = num_cells;
    uint32_t mid = 0, mid_key = 0;
    while (low != high) {
        mid = (low + high) / 2;
        mid_key = *((uint32_t *)leaf_node_key(node, mid));
        if (mid_key == key) {
            cursor->cell_num = mid;
            return cursor;
        } else if (mid_key > key) {
            high = mid;
        } else {
            low = mid + 1;
        }
    }
    // 当前节点没有找到key， 返回正确位置（可insert)
    cursor->cell_num = low;
    return cursor;
}

/* 在cursor位置，插入一行数据 */
void leaf_node_insert(cursor_t *cursor, uint32_t key, row_t *row)
{
    void *node = get_page(cursor->table->pager, cursor->page_num);
    uint32_t num_cells = *leaf_node_num_cells(node);
    if (num_cells > LEAF_NODE_MAX_CELLS) {
        // 节点满
        leaf_node_split_and_insert(cursor, key, row);
        return;
    }
    if (cursor->cell_num < num_cells) {
        // 【】【cell_num】【】【】 在cell_num位置插入， 有序即向后移动
        for (uint32_t i = num_cells; i > cursor->cell_num; i--) {
             memcpy(leaf_node_cell(node, i), leaf_node_cell(node, i - 1),
                      LEAF_NODE_CELL_SIZE);
        }
    }
    *(leaf_node_num_cells(node)) += 1;
    *(uint32_t *)(leaf_node_key(node, cursor->cell_num)) = key;
    serialize_row(row, leaf_node_value(node, cursor->cell_num));
}

/**
 * @brief 叶子节点已满时，分割插入
 * 
 * @param cursor 
 * @param key 
 * @param val 
 * @details 1. 创建一个新next_leaf节点
 *  2.如果分裂节点是根叶子节点，则创建一个新的根内部节点
 *  3.如果分裂节点是非根的叶子节点，则指向父内部节点的insert
 */

void leaf_node_split_and_insert(cursor_t *cursor, uint32_t key, row_t *val)
{
    void *old_node= get_page(cursor->table->pager, cursor->page_num);
    uint32_t old_max = get_node_max_key(cursor->table->pager, old_node);
    // 1. 创建一个新叶子节点：
    uint32_t new_page_num = get_unused_page_num(cursor->table->pager);
    void *new_node = get_page(cursor->table->pager, new_page_num);
    initialize_leaf_node(new_node);
    *node_parent(new_node) = *node_parent(old_node);
    *leaf_node_next_leaf(new_node) = *leaf_node_next_leaf(old_node);
    *leaf_node_next_leaf(old_node) = new_page_num;

    // 已超满节点，此时个数为 LEAF_NODE_MAX_CELLS + 1
    // 1. 将每个cell 转移到新位置。 2。插入的cell无需转移
    for (int32_t i = LEAF_NODE_MAX_CELLS; i >= 0; i--) {
        void *destination_node;
        if (i >= LEAF_NODE_LEFT_SPLIT_COUNT) {
            destination_node = new_node;
        } else {
            destination_node = old_node;
        }
        uint32_t index_within_node = i % LEAF_NODE_LEFT_SPLIT_COUNT;
        void *destination = leaf_node_cell(destination_node, index_within_node);

        if (i == cursor->cell_num) {

            serialize_row(val, leaf_node_value(destination_node, index_within_node));
            *(uint32_t *)leaf_node_key(destination_node, index_within_node) = key;
        } else if (i > cursor->cell_num) {
            //
            memcpy(destination, leaf_node_cell(old_node, i - 1), LEAF_NODE_CELL_SIZE);
        } else {
            memcpy(destination, leaf_node_cell(old_node, i), LEAF_NODE_CELL_SIZE);
        }
    }

    // 更新分割后的节点
    *(leaf_node_num_cells(old_node)) = LEAF_NODE_LEFT_SPLIT_COUNT;
    *(leaf_node_num_cells(new_node)) = LEAF_NODE_RIGHT_SPLIT_COUNT;

    if (is_node_root(old_node)) {
        // 对根节点划分 （树起始）， 需要创建一个新的parent
        return create_new_root(cursor->table, new_page_num);
    } else {
        // 对叶子节点分割
        uint32_t parent_page_num = *node_parent(old_node);
        uint32_t new_max = get_node_max_key(cursor->table->pager, old_node);
        void *parent = get_page(cursor->table->pager, parent_page_num);
        // TODO ?
        // 1. 更新内部节点中的key， 2. 对新建的页（右侧），创建内部节点的cell
        update_internal_node_key(parent, old_max, new_max);
        internal_node_insert(cursor->table, parent_page_num, new_page_num);
        return;
    }
}

/**
 * @brief 移除cell
 * 
 * @param node 
 * @param cell_num 
 */
void leaf_node_remove_cell(void *node, uint32_t cell_num)
{
    for (size_t i = cell_num; i < *leaf_node_num_cells(node); i++) {
        memcpy(leaf_node_cell(node, i), leaf_node_cell(node, i+1), LEAF_NODE_CELL_SIZE);
    }
    *leaf_node_num_cells(node)-=1;   
    memset(leaf_node_cell(node, *leaf_node_num_cells(node)), '\0', LEAF_NODE_CELL_SIZE); 
}


/**
 * @brief 释放page
 * 
 * @param node 
 */
void free_node(void *node)
{
    memset(node, '\0', PAGE_SIZE);
    // TODO page回收
}

/**
 * @brief 合并两个叶子节点
 * 
 * @param node1 
 * @param node2 
 */
void* leaf_node_merge(void *node1, void *node2)
{
    void *dest_cell = leaf_node_cell(node1, *leaf_node_num_cells(node1));
    memcpy(dest_cell, leaf_node_cell(node2, 0), LEAF_NODE_CELL_SIZE * (*leaf_node_num_cells(node2)));

    *leaf_node_num_cells(node1) += *leaf_node_num_cells(node2);

    free_node(node2);
    return node1;
}
/**
 * @brief 移除cell, #cursor
 * 
 * @param cursor 
 */
void leaf_node_remove(void *node, cursor_t *cursor, uint32_t key)
{   
    void *remove_cell = leaf_node_cell(node, cursor->cell_num);
    uint32_t num_cells = *leaf_node_num_cells(node);
    if ((num_cells - 1 ) >= LEAF_NODE_MIN_CELLS || is_node_root(node)) {
        // 1. 不产生上溢，直接删除
        leaf_node_remove_cell(node, cursor->cell_num);
    } else {
        // 2. 产生下溢出:
        // 2.0 如果自己是最右边孩子：

        // 2.1 右兄弟有的借： 父下来，兄弟上去
        uint32_t right_bro_page_num = *leaf_node_next_leaf(node);
        void *right_bro = get_page(cursor->table->pager, right_bro_page_num);
        // node移除cell
        leaf_node_remove_cell(node, cursor->cell_num);
        // 右节点第一个cell推过来
        memcpy(leaf_node_cell(node, *leaf_node_num_cells(node)), leaf_node_cell(right_bro, 0), LEAF_NODE_CELL_SIZE);
        *leaf_node_num_cells(node) += 1;
        // 更新父节点cell的key

        // 右节点移除第一个cell
        leaf_node_remove_cell(right_bro, 0);

        // 2.2 没得借

    }
}

/**
 * @brief Create a new root object/*
* 树上升， 根节点上升，创建一个新的根。
* Old root copied to new page, becomes left child.
* Re-initialize root page to contain the new root node.
* New root node points to two children.
 * 
 * 
 * @param table 
 * @param right_child_page_num 
 */
void create_new_root(table_t *table, uint32_t right_child_page_num)
{
    void *root = get_page(table->pager, table->root_page_num);
    void *right_child = get_page(table->pager, right_child_page_num);
    uint32_t left_child_page_num = get_unused_page_num(table->pager);
    void *left_child = get_page(table->pager, left_child_page_num);

    if (get_node_type(root) == NODE_INTERNAL) {
        initialize_internal_node(right_child);
        initialize_internal_node(left_child);
    }

    /* 左孩子内容复制过来 */
    memcpy(left_child, root, PAGE_SIZE);
    set_node_root(left_child, false);

    if (get_node_type(left_child) == NODE_INTERNAL) {
        // TODO ?
        void *child;
        for (int i = 0; i < *internal_node_num_keys(left_child); ++i) {
            child = get_page(table->pager, *internal_node_child(left_child, i));
            *node_parent(child) = left_child_page_num;
        }
        child = get_page(table->pager, *internal_node_right_child(left_child));
        *node_parent(child) = left_child_page_num;
    }

    /* 初始化根节点 */
    initialize_internal_node(root);
    set_node_root(root, true);
    *internal_node_num_keys(root) = 1; //TODO
    *internal_node_child(root, 0) = left_child_page_num;
    uint32_t left_child_max_key = get_node_max_key(table->pager, left_child);
    *internal_node_key(root, 0) = left_child_max_key;
    *internal_node_right_child(root) = right_child_page_num;
    *node_parent(left_child) = table->root_page_num;
    *node_parent(right_child) = table->root_page_num;
    *node_parent(root) = INVALID_PAGE_NUM; // 根节点的父亲parent_page_num是非法的
}

void close_input_buffer(input_buffer_t* input_buffer);

/*==============cursor ====================*/
/* 创建一个cursor指向 表第一个记录。*/
cursor_t *table_start(table_t *table)
{
    // 返回最小的key数据 所在cursor
    cursor_t *cursor = table_find(table, 0);
    void *node = get_page(table->pager, cursor->page_num);
    uint32_t num_cells = *leaf_node_num_cells(node);
    cursor->end_of_table = (num_cells == 0);

    return cursor;
}
/* 创建一个cursor 指向表尾部*/
cursor_t *table_end(table_t *table)
{
    cursor_t *cursor = (cursor_t *) malloc(sizeof(cursor_t));
    cursor->table = table;
    cursor->page_num = table->root_page_num;
    // TODO 不对把
    void *root_node = get_page(table->pager, cursor->page_num);
    uint32_t num_cells = *leaf_node_num_cells(root_node);
    cursor->cell_num = 0;
    cursor->end_of_table = true;
    return cursor;
}

/**
 * @brief 找到key的位置，#pagenum#cellnum
 * 
 * @param table 
 * @param key 
 * @return cursor_t* 
 */
cursor_t *table_find(table_t *table, uint32_t key)
{
    uint32_t root_page_num = table->root_page_num;
    void *root_node = get_page(table->pager, root_page_num);

    if (get_node_type(root_node) == NODE_LEAF) {
        return leaf_node_find(table, root_page_num, key);
    } else {
        return internal_node_find(table, root_page_num, key);
    }
}
/* 总是假设 0..num_pages-1 已经被分配，*/
uint32_t get_unused_page_num(pager_t *pager)
{
    return pager->num_pages;
}

/*
 * 根据page_num获取page：0，1，2....
 * 如果没有命中缓存，从文件读取上来
 * 如果命中缓存，直接返回。
 * */
void *get_page(pager_t* pager, uint32_t page_num)
{
    if (page_num >= TABLE_MAX_PAGES) {
        printf("Tried to fetch page number out of bounds. %d > %d\n", page_num,
               TABLE_MAX_PAGES);
        exit(EXIT_FAILURE);
    }

    if (pager->pages[page_num] == NULL) {
        // 未命中缓存， 分配内存，从文件读
        void *page = malloc(PAGE_SIZE);
        uint32_t num_pages = pager->file_length / PAGE_SIZE; // 获取page个数

        // We might save a partial page at the end of the file.
        if (pager->file_length % PAGE_SIZE) {
            num_pages += 1;
        }

        if (page_num < num_pages) {
            lseek(pager->fd, page_num * PAGE_SIZE, SEEK_SET);
            ssize_t bytes_read = read(pager->fd, page, PAGE_SIZE);
            if (bytes_read == -1) {
                printf("Error reading file: %d\n", errno);
                exit(EXIT_FAILURE);
            }
        }
        pager->pages[page_num] = page;
        if (page_num >= pager->num_pages) {
            pager->num_pages = page_num + 1;
        }
    }
    return pager->pages[page_num];
}
/* 将page刷盘
 * 整页刷盘， cell不会跨page
 * */
void pager_flush(pager_t *pager, uint32_t page_num)
{
    if (pager->pages[page_num] == NULL) {
        printf("Tried to flush null page.\n");
        exit(EXIT_FAILURE);
    }
    off_t offset = lseek(pager->fd, page_num * PAGE_SIZE, SEEK_SET);
    if (offset == -1) {
        printf("Error seeking: %d\n", errno);
        exit(EXIT_FAILURE);
    }
    ssize_t bytes_written = write(pager->fd, pager->pages[page_num], PAGE_SIZE);
    if (bytes_written == -1) {
        printf("Error writing: %d\n", errno);
        exit(EXIT_FAILURE);
    }
}
/*
 * 1. 缓存刷新到盘
 * 2. 关闭db file
 * 3. 释放table， pager
 * */
void db_close(table_t *table)
{
    pager_t *pager = table->pager;
    for (uint32_t i = 0; i < pager->num_pages; ++i) {
        if (pager->pages[i] == NULL) {
            continue;
        }
        pager_flush(pager, i);
        free(pager->pages[i]);
        pager->pages[i] = NULL;
    }

    int res = close(pager->fd);
    if (res == -1) {
        printf("Error closing db file.\n");
        exit(EXIT_FAILURE);
    }

    for (uint32_t i = 0; i < TABLE_MAX_PAGES; ++i) {
        void *page = pager->pages[i];
        if (page) {
            free(page);
            pager->pages[i] = NULL;
        }
    }
    free(pager);
    free(table);
}
/* cursor 记录下移一行*/
void cursor_advance(cursor_t *cursor)
{
    uint32_t page_num = cursor->page_num;
    void *node = get_page(cursor->table->pager, page_num);
    cursor->cell_num += 1;
    if (cursor->cell_num >= *leaf_node_num_cells(node)) {
        // 跨节点
        uint32_t next_page_num = *leaf_node_next_leaf(node);
        if (next_page_num == 0) {
            cursor->end_of_table = true;
        } else {
            cursor->page_num = next_page_num;
            cursor->cell_num = 0;
        }
    }
}


// 通过cursor返回访问字节位置
void * cursor_value(cursor_t *cursor)
{
    uint32_t page_num = cursor->page_num;
    void* node = get_page(cursor->table->pager, page_num);
    return leaf_node_value(node, cursor->cell_num);
}

/* row_t 落入内存*/
void serialize_row(row_t* source, void* destination)
{
    memcpy(destination + ID_OFFSET, &(source->id), ID_SIZE);
    strncpy(destination + USER_NAME_OFFSET, source->user_name, USER_NAME_SIZE);
    strncpy(destination + EMAIL_OFFSET, source->email, EMAIL_SIZE);
}

void print_row(row_t *row)
{
    printf("(%d, %s, %s)\n", row->id, row->user_name, row->email);
}

/* 从内存读,  source 为 一行的起始地址*/
void deserialize_row(void* source, row_t* destination) {
    memcpy(&(destination->id), source + ID_OFFSET, ID_SIZE);
    memcpy(&(destination->user_name), source + USER_NAME_OFFSET, USER_NAME_SIZE);
    memcpy(&(destination->email), source + EMAIL_OFFSET, EMAIL_SIZE);
}

/* 执行select， 从table.page内存读出*/
execute_res execute_select(sql_statement * statement, table_t * table)
{
    row_t row;
    cursor_t *cursor = table_start(table);
    while (!cursor->end_of_table) {
        deserialize_row(cursor_value(cursor), &row);
        print_row(&row);
        cursor_advance(cursor);
    }
    free(cursor);
    return EXECUTE_SUCCESS;
}

/* 执行insert， 写入table.page内存*/
execute_res execute_insert(sql_statement* statement, table_t* table)
{
    void *node = get_page(table->pager, table->root_page_num);
    uint32_t num_cells = *((uint32_t *)leaf_node_num_cells(node));

    row_t* row = &(statement->row);
    uint32_t key = row->id;
    cursor_t *cursor = table_find(table, key);
    if (cursor->cell_num < num_cells) {
        uint32_t key_at_idx = *((uint32_t *) leaf_node_key(node, cursor->cell_num));
        if (key == key_at_idx) {
            return EXECUTE_DUPLICATE_KEY;
        }
    }

    leaf_node_insert(cursor, row->id, row);
    free(cursor);
    return EXECUTE_SUCCESS;
}

/**
 * @brief 通过id找到cell，请0文本，移除cell
 * 
 * @param statement 
 * @param table 
 * @return execute_res 
 */
execute_res execute_delete(sql_statement *statement, table_t *table)
{
    uint32_t id = statement->row.id;
    uint32_t key = id;

    cursor_t* cursor =table_find(table, key);

    // 1. 
    void *node = get_page(table->pager, cursor->page_num);
    assert(get_node_type(node) == NODE_LEAF);

    leaf_node_remove(node, cursor, key);

    return EXECUTE_SUCCESS;
}
/*
 * virtual machines
 * */
execute_res execute_statement(sql_statement *statement, table_t *table)
{
    switch (statement->type) {
        case STATEMENT_INSERT:
            return execute_insert(statement, table);
        case STATEMENT_SELECT:
            return execute_select(statement, table);
        case STATEMENT_DELETE:
            return execute_delete(statement, table);
    }
}

/*
 * 打开文件， 初始化pager
 * */
pager_t *pager_open(const char *file_name)
{
    // 文件读写， 文件所有者读写权限
    int fd = open(file_name, O_RDWR|O_CREAT, S_IWUSR|S_IRUSR);
    if (fd < 0) {
        printf("Unable to open file.\n");
        exit(EXIT_FAILURE);
    }

    off_t file_length = lseek(fd, 0, SEEK_END);
    pager_t *pager = (pager_t *) malloc(sizeof(pager_t));
    pager->fd = fd;
    pager->file_length = file_length;
    pager->num_pages = file_length / PAGE_SIZE; // 初始化为最大page个数
    if (file_length % PAGE_SIZE != 0) {
        printf("DB file is not a whole number of pages. corrupt file.\n");
        exit(EXIT_FAILURE);
    }

    for (uint32_t i = 0; i < TABLE_MAX_PAGES; ++i) {
        pager->pages[i] = NULL;
    }
    
    return pager;
}

/* db初始化
 * 1. 打开db file
 * 2. 初始化pager
 * 3. 初始化table
 * 4. 初始化page0 ， 根节点
 * */
table_t *db_open(const char *file_name)
{
    pager_t *pager = pager_open(file_name);
    uint32_t num_rows = pager->file_length / ROW_SIZE;

    table_t *table = (table_t *) malloc(sizeof(table_t));
    table->pager = pager;
    table->root_page_num = 0;

    if (pager->num_pages == 0) {
        // 分配page0 , 初始化根节点
        void *root_node = get_page(pager, 0);
        initialize_leaf_node(root_node);
        set_node_root(root_node, true);
    }
    return table;
}

// TODO extend pwd-mgr : web - name - pwd
/* 处理insert*/
prepare_res_type prepare_insert(input_buffer_t* input_buffer, sql_statement *statement)
{
    statement->type = STATEMENT_INSERT;
    char *keyword = strtok(input_buffer->buffer, " ");
    char *id_str = strtok(NULL, " ");
    char *username = strtok(NULL, " ");
    char *email = strtok(NULL, " ");

    if (id_str == NULL || username == NULL || email == NULL) {
        return PREPARE_SYNTAX_ERROR;
    }
    int id = atoi(id_str);
    if (id < 0) {
        return PREPARE_NEGATIVE_ID;
    }
    if (strlen(username) > COLUMN_USERNAME_SIZE) {
        return PREPARE_STRING_TOO_LONG;
    }
    if (strlen(email) > COLUMN_EMAIL_SIZE) {
        return PREPARE_STRING_TOO_LONG;
    }
    statement->row.id = id;
    strcpy(statement->row.user_name, username);
    strcpy(statement->row.email, email);
    return PREPARE_SUCCESS;
}

prepare_res_type prepare_delete(input_buffer_t *input_buffer, sql_statement *statement)
{
    statement->type = STATEMENT_DELETE;
    char *keyword = strtok(input_buffer->buffer, " ");
    char *id_str = strtok(NULL, " ");
    int id = atoi(id_str);
    if (id < 0) {
        return PREPARE_NEGATIVE_ID;
    }
    statement->row.id = id;
}
/* 填充 statement*/
/*
 * insert 1 cstack foo@bar.com
*/
prepare_res_type prepare_statement(input_buffer_t *input_buffer, sql_statement *statement)
{
    if (strncmp(input_buffer->buffer, "insert", 6) == 0) {
        return prepare_insert(input_buffer, statement);
    }
    if (strncmp(input_buffer->buffer, "select", 6) == 0) {
        statement->type = STATEMENT_SELECT;
        return PREPARE_SUCCESS;
    }
    if (strncmp(input_buffer->buffer, "delete", 6) == 0) {
        statement->type = STATEMENT_DELETE;
        return prepare_delete(input_buffer, statement);
    }
    return PREPARE_UNRECOGNIZED_STATEMENT;
}

/*处理源命令*/
meta_cmd_res do_meta_command(input_buffer_t *input_buffer, table_t *table)
{
    if (strcmp(input_buffer->buffer, ".exit") == 0) {
        close_input_buffer(input_buffer);
        db_close(table);
        exit(EXIT_SUCCESS);
    } else if (strcmp(input_buffer->buffer, ".constants") == 0) {
        printf("Constants:\n");
        print_constants();
        return META_COMMAND_SUCCESS;
    } else if (strcmp(input_buffer->buffer, ".btree") == 0) {
        printf("Tree:\n");
        print_tree(table->pager, 0, 0);
        return META_COMMAND_SUCCESS;
    } else if (strcmp(input_buffer->buffer, ".help") == 0) {
        printf("Help:\n");
        printf("Meta command:\n");
        printf("\t.exit [] exit\n");
        printf("\t.constants [] constants\n");
        printf("\t.btree [] btree\n");
        printf("\t.help [] help\n");

        printf("SQL command:\n");
        printf("\tselect [index][name][email] select\n");
        printf("\tinsert [] insert\n");

        return META_COMMAND_SUCCESS;
    } else {
        return META_COMMAND_UNRECOGNIZED_COMMAND;
    }
}

input_buffer_t* new_input_buffer()
{
    input_buffer_t* input_buffer = (input_buffer_t*)malloc(sizeof(input_buffer_t));
    input_buffer->buffer = NULL;
    input_buffer->buffer_length = 0;
    input_buffer->input_length = 0;

    return input_buffer;
}
void close_input_buffer(input_buffer_t* input_buffer)
{
    free(input_buffer->buffer);
    free(input_buffer);
}
void read_input(input_buffer_t *input_buffer)
{
    ssize_t bytes_read = getline(&(input_buffer->buffer), &(input_buffer->buffer_length), stdin);
    if (bytes_read < 0) {
        printf("Error reading input");
        exit(EXIT_FAILURE);
    }
    // 去除换行符号
    input_buffer->input_length = bytes_read - 1;
    input_buffer->buffer[bytes_read - 1] = '\0';
}
// 打印提示
void print_prompt()
{
    printf("db > ");
}
// 打印 Btree 配置
void print_constants() {
    printf("PAGE_SIZE: %d\n", PAGE_SIZE);
    printf("LEAF_NODE_CELL_SIZE: %d\n", LEAF_NODE_CELL_SIZE);
    printf("ROW_SIZE: %d\n", ROW_SIZE);
    printf("Internal node max cells : %d\n", INTERNAL_NODE_MAX_CELLS);
    printf("LEAF_NODE_MAX_CELLS: %d\n", LEAF_NODE_MAX_CELLS);
    printf("LEAF_NODE_MIN_CELLS: %d\n", LEAF_NODE_MIN_CELLS);
    
}
/**
 * @brief 打印table内存信息
 * 
 * @param table 
 */
void print_table(table_t * table)
{
    
}

void indent(uint32_t level)
{
    for (uint32_t i = 0; i < level; ++i) {
        printf("  ");
    }
}
// 打印BTREE
void print_tree(pager_t * pager, uint32_t page_num, uint32_t indentation_level)
{
    void *node = get_page(pager, page_num);
    uint32_t num_keys, child_page_num;

    switch (get_node_type(node)) {
        case NODE_INTERNAL:
            num_keys = *internal_node_num_keys(node);
            indent(indentation_level);
            printf("- Internal (num_keys %d) page_num (%d)\n", num_keys, page_num);
            if (num_keys > 0) {
                for (uint32_t  i = 0; i < num_keys; ++i) {
                    // numkey 和 入参？？ TODO
                    child_page_num = *internal_node_child(node, i);
                    print_tree(pager, child_page_num, indentation_level + 1);
                    indent(indentation_level + 1);
                    printf("- Internal cell_(%d) key %d\n", i, *internal_node_key(node, i));
                }
                child_page_num = *internal_node_right_child(node);
                print_tree(pager, child_page_num, indentation_level + 1);
            }
            break;
        case NODE_LEAF:
            num_keys = *leaf_node_num_cells(node);
            indent(indentation_level);
            printf("- Leaf (num_cells %d) page_num (%d)\n", num_keys, page_num);
            for (uint32_t i = 0; i < num_keys; ++i) {
                indent(indentation_level + 1);
                printf("- Leaf cell_(%d) key %u\n", i,*(uint32_t *)leaf_node_key(node, i));
            }
            break;
    }

}



/**
 *

 ~ sqlite3
SQLite version 3.16.0 2016-11-04 19:09:39
Enter ".help" for usage hints.
Connected to a transient in-memory database.
Use ".open FILENAME" to reopen on a persistent database.
sqlite> create table users (id int, username varchar(255), email varchar(255));
sqlite> .tables
users
sqlite> .exit
 * @param argc
 * @param argv
 * @return
 */

int main(int argc, char *argv[])
{
    if (argc < 2) {
        printf("Must supply a  db filename.\n");
        exit(EXIT_FAILURE);
    }
    char *file_name = argv[1];
    table_t *table = db_open(file_name);
    input_buffer_t *input_buffer = new_input_buffer();
    //printf("PAGES: %u, ROWS-PAGE: %u, ROW-SIZE :%u\n", TABLE_MAX_PAGES, ROWS_PER_PAGE, ROW_SIZE);
    while (true) {
        /*1. Making a Simple REPL*/
        print_prompt();
        read_input(input_buffer);
        if (input_buffer->buffer[0] == '.') {
            // 源命令
            switch (do_meta_command(input_buffer, table)) {
                case META_COMMAND_SUCCESS:
                    continue;
                case META_COMMAND_UNRECOGNIZED_COMMAND:
                    printf("Unrecognized command `%s` \n", input_buffer->buffer);
                    continue;
            }
        }
        sql_statement statement;
        switch (prepare_statement(input_buffer, &statement)) {
            // 拿到SQL 就去处理
            case PREPARE_SUCCESS:
                break;
            case PREPARE_SYNTAX_ERROR:
                printf("Syntax error. Could not parse statement.\n");
                continue;
            case PREPARE_UNRECOGNIZED_STATEMENT:
                printf("Unrecognized SQL command `%s` \n", input_buffer->buffer);
                continue;
            case PREPARE_STRING_TOO_LONG:
                printf("String is too long.\n");
                continue;
            case PREPARE_NEGATIVE_ID:
                printf("ID must be positive.\n");
                continue;
        }

        switch (execute_statement(&statement, table)) {
            case EXECUTE_SUCCESS:
                printf("Executed.\n");
                break;
            case EXECUTE_TABLE_FULL:
                printf("Error: Table full.\n");
                break;
            case EXECUTE_DUPLICATE_KEY:
                printf("Error: Duplicate key.\n");
                break;
        }
    }
    return 0;
}
