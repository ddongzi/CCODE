//
// Created by dong on 3/30/24.
//
#include <string.h>
#include "ssl.h"
int fill_ssl_header(uint8_t *msg, size_t size, ssl_header_t * header)
{
    if (size > 3) {
        return SSL_ERROR;
    }

    header->len = msg[0];
    header->version = (msg[1] >> 4) & 0x0f;
    header->type = msg[1] & 0x0f;
    header->time = msg[2];

    return SUCCESS;
}
int fill_ssl_body(uint8_t *msg, ssl_header_t * header, ssl_body_t * body)
{
    uint8_t *temp_msg = msg + 3;
    ssl_body_t *ret = memcpy(body, temp_msg, header->len);
    if (ret != body) {
        return NOR_ERROR;
    }
    return SUCCESS;

}