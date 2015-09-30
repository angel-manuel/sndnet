/**
 * @file
 * Provides functions for managing SecondNet messages
 * */

#ifndef SN_MSG_H_
#define SN_MSG_H_

#include "address.h"
#include "message_type.h"
#include "realaddress.h"

#include <stdint.h>
#include <sys/socket.h>

#ifdef __cplusplus
extern "C" {
#endif/**/

/**
 * Default message TTL(Time To Live)
 * */
#define SN_MSG_DEFAULT_TTL 32

/**
 * Max message payload length(should be quite under the ethernet MTU)
 * */
#define SN_MSG_MAX_LEN 1000

/**
 * Message header as used on real messages
 * */
typedef struct sn_header_t_ sn_header_t;

/**
 * Holds an message(header + payload). Usually malloc'd
 * */
typedef struct sn_message_t_ sn_message_t;

/**
 * Reads a message from a given socket. Allocates memory.
 * @param socket_fd Socket file descriptor
 * @param[out] rem_addr For storage of the sender network address
 * @return A new message(must be freed) or NULL if error
 */
sn_message_t* sn_message_recv(int socket_fd, sn_realaddr_t* rem_addr);

/**
 * Creates a message ready for sending
 * @param dst SecondNet destination address(copied)
 * @param src SecondNet source address(copied)
 * @param type Message type
 * @param len Payload length
 * @param payload Pointer to payload(copied)
 * @return A new message(must be freed) or NULL if error
 * */
sn_message_t* sn_message_pack(const sn_addr_t* dst, const sn_addr_t* src, sn_message_type_t type, size_t len, const char* payload);

/**
 * Sends a message(low-level)
 * @param msg The message to be sent
 * @param socket_fd The file descriptor of the datagram socket to be used
 * @param rem_addr Pointer to the destination address
 * @return Number of characters sent or -1 if ERROR
 * */
int sn_message_send(const sn_message_t* msg, int socket_fd, const sn_realaddr_t* rem_addr);

/*Struct definition*/

struct sn_header_t_ {
    unsigned char dst[SN_ADDR_LEN]; /**< SecondNet destination address */
    unsigned char src[SN_ADDR_LEN]; /**< SecondNet source address(spoofable) */
    uint16_t ttl; /**< TTL(Time To Live), decreased when forwarded, when 0 message isnt forwarded any more */
    sn_message_type_t type; /**< Message type */
    uint16_t len; /**< Length of the payload */
};

struct sn_message_t_ {
    sn_header_t header; /**< Header part */
    unsigned char payload[0]; /**< O-length array used to represent the variable size payload */
};

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif/*SN_MSG_H_*/
