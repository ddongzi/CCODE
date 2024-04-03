//
// Created by dong on 3/30/24.
//
// TODO 为啥这里cmake不行？
#include "ssl.h"
int main(void )
{
    printf(SSL_HEADER_TO_STRING(10, 11 ,1));
    ssl_header_t *header = malloc(sizeof(ssl_header_t));
    return 0;
}