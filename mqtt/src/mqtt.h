#ifndef MQTT_H
#define MQTT_H



#include <stdio.h>

/*
packet :
- fixed header
- variable header (optional)
- payload

*/

/*
# Fixed Header
    | Bit    | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 |
    |--------|---------------|---------------|
    | Byte 1 | MQTT type     |  Flags        |
    |--------|-------------------------------|
    | Byte 2 |                               |
    |  .     |      Remaining Length         |
    |  .     |                               |
    | Byte 5 |                               |
*/

#define MQTT_HEADER_LEN 2  // TODO
#define MQTT_ACK_LEN 4  // TODO

/* mqtt type byte ... */
#define CONNACK_BYTE 0X20
#define PUBLISH_BYTE 0X30

/* message type */
enum packet_type {
    CONNECT = 1,
    CONNACK,
    PUBLISH,
    PUBACK,
    PUBREC,
    PUBREL,
    SUBSCRIBE,
    SUBACK,
    UNSUBSCRIBE,
    UNSUBACK,
    PINGREQ,
    PINGRESP,
    DISCONNECT
};

/* flag : qos_level */
enum qos_level {AT_MOST_ONCE};

/* mqtt header*/
union mqtt_header {
    unsigned char byte;
    struct {
        unsigned retain : 1; // 保留消息
        unsigned  qos : 2;
        unsigned dup : 1;
        unsigned type : 4;
    } bits;
};

/* mqtt_ connect packet*/
struct mqtt_connect {
    union mqtt_header header;
    // viriable header
    union {
        unsigned char byte;
        struct {
            unsigned reserved : 1;
            unsigned clean_session : 1; // 是否强制创建一个新的session
            unsigned will : 1; // 是否设置了遗嘱消息。 客户端断连时候设置
            unsigned will_qos : 2; // 遗嘱消息等级
            unsigned will_retain : 1; // 遗嘱消息是否保留
            unsigned password : 1; // 客户端提供密码链接
            unsigned username : 1;
        };
    };
    struct {
        unsigned short keepalive; // 连接保活周期
        unsigned char* client_id;
        unsigned char* username;
        unsigned char* password;
        unsigned char* will_topic;
        unsigned char* will_message;
    } payload;
};

struct mqtt_connack {
    union mqtt_header header;
    union {
        unsigned char byte;
        struct {
            unsigned session_present : 1;
            unsigned reserved : 7;
        } bits;
    };
    unsigned char rc;
};

struct mqtt_subscribe {
    union mqtt_header header;
    unsigned short pkt_id;
    unsigned short tuples_len;
    struct {
        unsigned short topic_len;
        unsigned short *topic;
        unsigned qos;
    } *tuples;
};

struct mqtt_unsubscribe {
    union mqtt_header header;
    unsigned short pkt_id;
    unsigned short tuples_len;
    struct {
        unsigned short topic_len;
        unsigned short *topic;
    } *tuples;
};

struct mqtt_suback {
    union mqtt_header header;
    unsigned short pkt_id;
    unsigned short rcslen; //TODO
    unsigned char *rcs;
};

struct mqtt_publish {
    union mqtt_header header;
    unsigned short pkt_id;
    unsigned short topiclen;
    unsigned char *topic;
    unsigned short payloadlen;
    unsigned char *payload;
};

struct mqtt_ack {
    union mqtt_header header;
    unsigned short pkt_id;
};

typedef struct mqtt_ack mqtt_puback;
typedef struct mqtt_ack mqtt_pubrec;
typedef struct mqtt_ack mqtt_pubrel;
typedef struct mqtt_ack mqtt_pubcomp;
typedef struct mqtt_ack mqtt_unsuback;
typedef union mqtt_header mqtt_pingreq;
typedef union mqtt_header mqtt_pingresp;
typedef union mqtt_header mqtt_disconnect;

union mqtt_packet {
    struct mqtt_ack ack;
    union mqtt_header header;
    struct mqtt_connect connect;
    struct mqtt_connack connack;
    struct mqtt_suback suback;
    struct mqtt_publish publish;
    struct mqtt_subscribe subscribe;
    struct mqtt_unsubscribe unsubscribe;
};

/* mqtt_packet 解码编码为 某一类型结构体*/
int mqtt_encode_length(unsigned char *, size_t);
unsigned long long mqtt_decode_length(const char **);
int unpack_mqtt_packet(const unsigned char *, union mqtt_packet *);
unsigned char *pack_mqtt_packet(const union mqtt_packet *, unsigned);

union mqtt_header *mqtt_packet_header(unsigned char);
struct mqtt_ack *mqtt_packet_ack(unsigned char , unsigned short);
struct mqtt_connack *mqtt_packet_connack(unsigned char, unsigned char, unsigned char);
struct mqtt_suback *mqtt_packet_suback(unsigned char, unsigned short,
                                       unsigned char *, unsigned short);
struct mqtt_publish *mqtt_packet_publish(unsigned char, unsigned short, size_t,
                                         unsigned char *, size_t, unsigned char *);
void mqtt_packet_release(union mqtt_packet *, unsigned);

































#endif