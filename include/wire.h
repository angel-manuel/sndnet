/**
 * @file
 * Contains the wire headers
 * */

#ifndef SN_WIRE_H_
#define SN_WIRE_H_

#include "common.h"
#include "net/addr.h"
#include "io/naddr.h"
#include "crypto/sign.h"

#include <stdint.h>

/*******************************************************************
    net/packet header

     0               1               2               3
     0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  0 |         Payload Length        |       Time To Live(TTL)       |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  4 |                                                               |
    +                                                               +
  8 |                                                               |
    +                                                               +
 12 |                                                               |
    +                                                               +
 16 |                                                               |
    +                  Destination SecondNet address                +
 20 |                                                               |
    +                                                               +
 24 |                                                               |
    +                                                               +
 28 |                                                               |
    +                                                               +
 32 |                                                               |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 36 |                                                               |
    +                                                               +
 40 |                                                               |
    +                                                               +
 44 |                                                               |
    +                                                               +
 48 |                                                               |
    +                    Source SecondNet address                   +
 52 |                                                               |
    +                                                               +
 56 |                                                               |
    +                                                               +
 60 |                                                               |
    +                                                               +
 64 |                                                               |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

********************************************************************/

#define SN_WIRE_NET_HEADER_SIZE 68

typedef struct {
    uint16_t len; /**< Length of the payload */
    uint16_t ttl; /**< TTL(Time To Live), decreased when forwarded, when 0 message isnt forwarded any more */
    sn_net_addr_ser_t dst; /**< SecondNet destination address */
    sn_net_addr_ser_t src; /**< SecondNet source address */
} sn_wire_net_header_t;

SN_ASSERT_COMPILE(sizeof(sn_wire_net_header_t) == SN_WIRE_NET_HEADER_SIZE);

/*******************************************************************
    crypto/msg header

     0               1               2               3
     0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

********************************************************************/

#define SN_WIRE_CRYPTO_HEADER_SIZE 1

typedef struct {
    uint8_t TODO;
} sn_wire_crypto_header_t;

SN_ASSERT_COMPILE(sizeof(sn_wire_crypto_header_t) == SN_WIRE_CRYPTO_HEADER_SIZE);

#endif/*SN_WIRE_H_*/
