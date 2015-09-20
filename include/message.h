#ifndef SNDNET_MESSAGE_H_
#define SNDNET_MESSAGE_H_

#include "address.h"

#include <stdint.h>
#include <sys/socket.h>

#ifdef __cplusplus
extern "C" {
#endif/**/

#define SNDNET_MESSAGE_MAX_LENGTH 1000

typedef struct sndnet_header_t_ sndnet_header_t;

typedef struct sndnet_message_t_ sndnet_message_t;

sndnet_message_t* sndnet_message_recv(int socket_fd);

/*Struct definition*/

struct sndnet_header_t_ {
    unsigned char dst[SNDNET_ADDRESS_LENGTH];
    unsigned char src[SNDNET_ADDRESS_LENGTH];
    uint16_t ttl;
    uint16_t len;
};

struct sndnet_message_t_ {
    struct sockaddr rem_addr;
    sndnet_header_t header;
    unsigned char payload[0];
};

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif/*SNDNET_MESSAGE_H_*/