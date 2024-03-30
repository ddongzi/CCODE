//
// Created by dong on 3/30/24.
//

#ifndef MY_BASE_H
#define MY_BASE_H

#define MEMBER_OF(type, member) type->member


typedef enum {
    SUCCESS,
    NOR_ERROR,
    SSL_ERROR
} ret_code_t;

#endif //MY_BASE_H
