/**
 * @file
 * Provides functions for managing SecondNet messages
 * */

#ifndef SNDNET_MESSAGE_H_
#define SNDNET_MESSAGE_H_

#include "address.h"
#include "realaddress.h"

#include <stdint.h>
#include <sys/socket.h>

#ifdef __cplusplus
extern "C" {
#endif/**/

/**
 * Default message TTL(Time To Live)
 * */
#define SNDNET_MESSAGE_DEFAULT_TTL 32

/**
 * Max message payload length(should be quite under the ethernet MTU)
 * */
#define SNDNET_MESSAGE_MAX_LENGTH 1000

/**
 * Message header as used on real messages
 * */
typedef struct sndnet_header_t_ sndnet_header_t;

/**
 * Holds an message(header + payload). Usually malloc'd
 * */
typedef struct sndnet_message_t_ sndnet_message_t;

/**
 * Reads a message from a given socket. Allocates memory.
 * @param socket_fd Socket file descriptor
 * @param[out] rem_addr For storage of the sender network address
 * @return A new message(must be freed) or NULL if error
 */
sndnet_message_t* sndnet_message_recv(int socket_fd, sndnet_realaddr_t* rem_addr);

/**
 * Creates a message ready for sending
 * @param dst SecondNet destination address(copied)
 * @param src SecondNet source address(copied)
 * @param len Payload length
 * @param payload Pointer to payload(copied)
 * @return A new message(must be freed) or NULL if error
 * */
sndnet_message_t* sndnet_message_pack(const sndnet_addr_t* dst, const sndnet_addr_t* src, size_t len, const char* payload);

int sndnet_message_send(const sndnet_message_t* msg, int socket_fd, const sndnet_realaddr_t* rem_addr);

/*Struct definition*/

struct sndnet_header_t_ {
    unsigned char dst[SNDNET_ADDRESS_LENGTH];
    unsigned char src[SNDNET_ADDRESS_LENGTH];
    uint16_t ttl;
    uint16_t len;
};

struct sndnet_message_t_ {
    sndnet_header_t header;
    unsigned char payload[0];
};

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif/*SNDNET_MESSAGE_H_*/