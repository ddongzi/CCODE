//
// Created by dong on 3/30/24.
//

#ifndef MY_BASE_H
#define MY_BASE_H

#include <assert.h>

#define MEMBER_OF(type, member) type->member

// 2024-4-4 0:0:0 
#define BASE_TIME_STAMP 1712160000

#define KEY_FILE "/home/dong/ALLCODE/develop/conf/server.key" // 私钥
#define CERT_FILE "/home/dong/ALLCODE/develop/conf/server.crt" // 服务器数字证书
#define CA_FILE "ca.crt" // 根证书
#define CRL_FILE "crl.pem"  // 吊销列表

#define MAX_BUF 256

#define CHECK(x) assert((x) >= 0)


typedef enum {
    SUCCESS,
    NOR_ERROR,
    SSL_ERROR
} ret_code_t;

#endif //MY_BASE_H
