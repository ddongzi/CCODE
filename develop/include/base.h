//
// Created by dong on 3/30/24.
//

#ifndef MY_BASE_H
#define MY_BASE_H

#define MEMBER_OF(type, member) type->member

// 2024-4-4 0:0:0 
#define BASE_TIME_STAMP 1712160000

typedef enum {
    SUCCESS,
    NOR_ERROR,
    SSL_ERROR
} ret_code_t;

#endif //MY_BASE_H
