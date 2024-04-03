//
// Created by dong on 3/29/24.
//

#ifndef SSL_SSL_H
#define SSL_SSL_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h
#include "base.h"

#define MAX_METHOD_LENGTH 10
#define MAX_HEADER_LENGTH 100
#define MAX_BODY_LENGTH 1024
#pragma pack(1)
typedef struct {
    unsigned int len : 8; // body的长度
    unsigned int version : 4; // 版本号，占4位
    unsigned int type : 4; // 记录类型，占3位
    unsigned int time : 8; // 时间，占8位
} ssl_header_t;

typedef struct {
    ssl_header_t header;
    uint8_t body[0];
    size_t size;
} ssl_body_t;

// 定义宏，用于将 ssl_header_t 结构体转换为字符串
//TODO 如何在宏定义中将结构体展开？
#define SSL_HEADER_TO_STRING(len, type, version)  "ssl_header_t: len=" #len

int fill_ssl_header(uint8_t *msg, size_t size, ssl_header_t * header);
int fill_ssl_body(uint8_t *msg, ssl_header_t * header, ssl_body_t * body);

#endif //SSL_SSL_H
