/**
 * @file
 * Provides functions for managing SecondNet messages
 * */

#ifndef SN_PACKET_H_
#define SN_PACKET_H_

#include "addr.h"
#include "io/naddr.h"
#include "io/sock.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif/**/

/**
 * Default message TTL(Time To Live)
 * */
#define SN_PACKET_DEFAULT_TTL 32

/**
 * Max message payload length(should be quite under the ethernet MTU)
 * */
#define SN_PACKET_MAX_LEN 1000

#define SN_PACKET_PRINTABLE_LEN (25 + 3*5 + 2*SN_ADDR_PRINTABLE_LEN)

/**
 * Message header as used on real messages
 * */
typedef struct sn_packet_header_t_ sn_packet_header_t;

/**
 * Holds an message(header + payload). Usually malloc'd
 * */
typedef struct sn_packet_t_ sn_packet_t;

/**
 * Reads a message from a given socket. Allocates memory.
 * @param socket Socket
 * @param[out] src_addr For storage of the sender network address.
 * @return A new message(must be freed) or NULL if error
 */
sn_packet_t* sn_packet_recv(sn_io_sock_t socket, sn_io_naddr_t* src_addr);

/**
 * Creates a message ready for sending
 * @param dst SecondNet destination address(copied)
 * @param src SecondNet source address(copied)
 * @param type Message type
 * @param len Payload length
 * @param payload Pointer to payload(copied)
 * @return A new message(must be freed) or NULL if error
 * */
sn_packet_t* sn_packet_pack(const sn_addr_t* dst, const sn_addr_t* src, size_t len, const char* payload);

/**
 * Sends a message(low-level)
 * @param msg The message to be sent
 * @param socket Socket
 * @param dst_addr Pointer to the destination address
 * @return Number of characters sent or -1 if ERROR
 * */
int sn_packet_send(const sn_packet_t* msg, sn_io_sock_t socket, const sn_io_naddr_t* dst_addr);

/**
 * Gets the message destination
 * @param msg Message
 * @param[out] out_dst Where to store the destination
 * */
void sn_packet_get_dst(const sn_packet_t* msg, sn_addr_t* out_dst);

/**
 * Gets the message source
 * @param msg Message
 * @param[out] out_src Where to store the source
 * */
void sn_packet_get_src(const sn_packet_t* msg, sn_addr_t* out_src);

/**
 * Creates an string representation of a message header
 * @param msg Message
 * @param[out] out_str Pointer to buffer where string representation should be stored. Must have at least SN_PACKET_PRINTABLE_LEN size allocated.
 * */
void sn_packet_header_to_str(const sn_packet_t* msg, char* out_str);

/*Struct definition*/

struct sn_packet_header_t_ {
    sn_addr_ser_t dst; /**< SecondNet destination address */
    sn_addr_ser_t src; /**< SecondNet source address(spoofable) */
    uint16_t ttl; /**< TTL(Time To Live), decreased when forwarded, when 0 message isnt forwarded any more */
    uint16_t len; /**< Length of the payload */
};

struct sn_packet_t_ {
    sn_packet_header_t header; /**< Header part */
    unsigned char payload[0]; /**< O-length array used to represent the variable size payload */
};

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif/*SN_PACKET_H_*/
