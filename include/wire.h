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
  0 |         Payload Length        |      TTL      |               |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+               +
  4 |                                                               |
    +                                                               +
  8 |                                                               |
    +                                                               +
 12 |                                                               |
    +                                                               +
 16 |                                                               |
    +                    Source SecondNet address                   +
 20 |                                                               |
    +                                                               +
 24 |                                                               |
    +                                                               +
 28 |                                                               |
    +                                               +-+-+-+-+-+-+-+-+
 32 |                                               |               |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+               +
 36 |                                                               |
    +                                                               +
 40 |                                                               |
    +                                                               +
 44 |                                                               |
    +                                                               +
 48 |                                                               |
    +                                                               +
 52 |                                                               |
    +                                                               +
 56 |                                                               |
    +                                                               +
 60 |                                                               |
    +                                                               +
 64 |                                                               |
    +                   Ed25519-SHA512 Signature                    +
 68 |               (Everything below this is signed)               |
    +                                                               +
 72 |                                                               |
    +                                                               +
 76 |                                                               |
    +                                                               +
 80 |                                                               |
    +                                                               +
 84 |                                                               |
    +                                                               +
 88 |                                                               |
    +                                                               +
 92 |                                                               |
    +                                               +-+-+-+-+-+-+-+-+
 96 |                                               |               |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+               +
100 |                                                               |
    +                                                               +
104 |                                                               |
    +                                                               +
108 |                                                               |
    +                                                               +
112 |                                                               |
    +                 Destination SecondNet address                 +
116 |                                                               |
    +                                                               +
120 |                                                               |
    +                                                               +
124 |                                                               |
    +                                               +-+-+-+-+-+-+-+-+
128 |                                               |      Type     |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

********************************************************************/

#define SN_WIRE_NET_HEADER_SIZE 132

typedef struct {
    uint16_t len; /**< Length of the payload */
    uint8_t ttl; /**< TTL(Time To Live), decreased when forwarded, when 0 message isnt forwarded any more */
    sn_net_addr_ser_t src; /**< SecondNet source address */
    sn_crypto_sign_t sign; /**< Ed25519-SHA512 Signature */
    sn_net_addr_ser_t dst; /**< SecondNet destination address */
    uint8_t type; /**< Content type */
} sn_wire_net_header_t;

SN_ASSERT_COMPILE(sizeof(sn_wire_net_header_t) == SN_WIRE_NET_HEADER_SIZE);
SN_ASSERT_COMPILE(offsetof(sn_wire_net_header_t, len) == 0);

//net/packet types

typedef enum {
    SN_WIRE_NET_TYPE_USER = 0,
    SN_WIRE_NET_TYPE_PING = 1,
    SN_WIRE_NET_TYPE_PONG = 2,
    SN_WIRE_NET_TYPES
} sn_wire_net_type_t;

SN_ASSERT_COMPILE(SN_WIRE_NET_TYPES <= UINT8_MAX + 1);

#endif/*SN_WIRE_H_*/
