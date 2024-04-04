//
// Created by dong on 3/30/24.
//
#include <string.h>
#include "ssl.h"
int fill_ssl_header(uint8_t *msg, size_t size, ssl_header_t * header)
{
    header->len = msg[0];
    header->version = (msg[1] >> 4) & 0x0f;
    header->type = msg[1] & 0x0f;
    header->time = (unsigned int)msg[2];
    return SUCCESS;
}
int fill_ssl_body(uint8_t *msg_body, ssl_header_t * header, ssl_body_t * body)
{
    void *ret = memcpy(body->data, msg_body, header->len);
    if (ret != body->data) {
        return NOR_ERROR;
    }
    body->size = header->len;
    return SUCCESS;

}
