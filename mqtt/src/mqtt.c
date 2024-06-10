#include <stdlib.h>
#include <string.h>
#include "mqtt.h"
#include "pack.h"

static size_t unpack_mqtt_connect(const unsigned char *, union mqtt_header *, union mqtt_packet *);
static size_t unpack_mqtt_publish(const unsigned char *, union mqtt_header *, union mqtt_packet *);
static size_t unpack_mqtt_subscribe(const unsigned char *, union mqtt_header *, union mqtt_packet *);
static size_t unpack_mqtt_unsubscribe(const unsigned char *, union mqtt_header *, union mqtt_packet *);
static size_t unpack_mqtt_ack(const unsigned char *, union mqtt_header *, union mqtt_packet *);
static unsigned char *pack_mqtt_header(const union mqtt_header *);
static unsigned char *pack_mqtt_ack(const union mqtt_packet *);
static unsigned char *pack_mqtt_connack(const union mqtt_packet *);
static unsigned char *pack_mqtt_suback(const union mqtt_packet *);
static unsigned char *pack_mqtt_publish(const union mqtt_packet *);

// Remaining Length on fix header is at most 4 .
static const int MAX_LEN_BYTES = 4;

/* encode remaining length*/
/**
 * len : remaining length
 * return ： 占用字节
*/
int mqtt_encode_length(unsigned char *buf, size_t len)
{
    int bytes = 0;
    do {
        if (bytes + 1 > MAX_LEN_BYTES)
            return bytes;
        unsigned char d = len % 128; // 当前部分的值
        len /= 128;
        if (len > 0)
            d |= 128; // 后续还有值， 将最高位设置为1
        buf[bytes++] = d;
    } while (len > 0);
    return bytes;
}

/* 从buf 中解码remain length*/
unsigned long long mqtt_decode_length(const char **buf)
{
    unsigned long long value = 0LL;
    unsigned char c;
    int shift = 0;
    do {
        c = **buf;
        value += (c & 127) << shift;
        shift += 7;
    } while ((c & 128) != 0);
    return value;
}

static size_t unpack_mqtt_connect(const unsigned char *buf, 
    union mqtt_header *header, union mqtt_packet *pkt)
{
    struct mqtt_connect connect = {.header = *header};
    pkt->connect = connect;
    const unsigned char *init = buf;
    /* TODO type?*/

    /* read remain length */
    size_t len = mqtt_decode_length(buf);

    /* now, ignore protocol check*/
    buf = init + 8;
    /* read connect flags*/
    pkt->connect.byte = unpack_u8((uint8_t **)&buf);
    /* read keeplive*/
    pkt->connect.payload.keepalive = unpack_u16((uint8_t **)&buf);
    /* read client id length*/
    uint16_t cid_len = unpack_u16((uint8_t **)&buf);
    /* read client id*/
    if (cid_len > 0) {
        pkt->connect.payload.client_id = malloc(cid_len + 1);
        unpack_bytes(&buf, cid_len, pkt->connect.payload.client_id);
    }
    /* Read the will topic and message if will is set on flags */
    if (pkt->connect.bits.will == 1) {
        unpack_string16(&buf, &pkt->connect.payload.will_topic);
        unpack_string16(&buf, &pkt->connect.payload.will_message);
    }
    /* Read the username if username flag is set */
    if (pkt->connect.bits.username == 1)
        unpack_string16(&buf, &pkt->connect.payload.username);
    /* Read the password if password flag is set */
    if (pkt->connect.bits.password == 1)
        unpack_string16(&buf, &pkt->connect.payload.password);
    return len;
}

static size_t unpack_mqtt_subscribe(const unsigned char *buf,
    union mqtt_header *hdr,
    union mqtt_packet *pkt)
{
    struct mqtt_subscribe subscribe = {.header = *hdr};
    size_t len = mqtt_decode_length(&buf);
    size_t remaining_bytes = len;

    subscribe.pkt_id = unpack_u16(&buf);
    remaining_bytes -= sizeof(uint16_t);

    int cnt = 0;
    while (remaining_bytes > 0) {
        remaining_bytes -= sizeof(uint16_t);
        subscribe.tuples = realloc(subscribe.tuples, (cnt + 1) * sizeof(*subscribe.tuples)); 
        subscribe.tuples[cnt].topic_len = unpack_string16(&buf, &subscribe.tuples[cnt].topic);
        remaining_bytes -= subscribe.tuples[cnt].topic_len; 
        subscribe.tuples[cnt].qos = unpack_u8(&buf);
        remaining_bytes -= sizeof(uint8_t);
        cnt++;   
    }
    subscribe.tuples_len = cnt;
    pkt->subscribe = subscribe;
    return len;
}

static size_t unpack_mqtt_unsubscribe(const unsigned char *buf,
    union mqtt_header *hdr,
    union mqtt_packet *pkt)
{
    struct mqtt_unsubscribe unsubscribe = {.header = *hdr};
    size_t len = mqtt_decode_length(&buf);
    size_t remaining_bytes = len;
    unsubscribe.pkt_id = unpack_u16(&buf);
    remaining_bytes -= sizeof(uint16_t);

    int cnt = 0;
    while (remaining_bytes > 0) {
        remaining_bytes -= sizeof(uint16_t);
        unsubscribe.tuples = realloc(unsubscribe.tuples, (cnt + 1) * sizeof(*subscribe.tuples)); 
        unsubscribe.tuples[cnt].topic_len = unpack_string16(&buf, &subscribe.tuples[cnt].topic);
        remaining_bytes -= unsubscribe.tuples[cnt].topic_len; 
        cnt++;   
    }
    unsubscribe.tuples_len = cnt;
    pkt->unsubscribe = unsubscribe;
    return len;
}

static size_t unpack_mqtt_ack(const unsigned char *buf,
                              union mqtt_header *hdr,
                              union mqtt_packet *pkt) 
{
    struct mqtt_ack ack = { .header = *hdr };
    size_t len = mqtt_decode_length(&buf);
    ack.pkt_id = unpack_u16(&buf);
    pkt->ack = ack;
    return len;
}

// 
typedef size_t mqtt_unpack_handler(const unsigned char *buf,
                              union mqtt_header *hdr,
                              union mqtt_packet *pkt);
// message type
static mqtt_unpack_handler *unpack_handler[11] = {
    NULL,
    unpack_mqtt_connect,
    NULL,
    unpack_mqtt_publish,
    unpack_mqtt_ack,
    unpack_mqtt_ack,
    unpack_mqtt_ack,
    unpack_mqtt_ack,
    unpack_mqtt_subscribe,
    NULL,
    unpack_mqtt_unsubscribe
};

int unpack_mqtt_packet(const unsigned char *buf, union mqtt_packet *pkt)
{
    int rc = 0;
    unsigned char type = *buf;
    union mqtt_header header = {.byte = type};
    // 简单消息只需要处理fix header， 没有可变头部或者负载
    if (header.bits.type == DISCONNECT ||
        header.bits.type == PINGREQ ||
        header.bits.type == PINGRESP
        )
        pkt->header = header;
    else
        rc = unpack_handler[header.bits.type](++buf, &header, pkt);
    return rc;       
}