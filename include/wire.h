/**
 * @file
 * Contains the wire headers
 * */

#ifndef SN_WIRE_H_
#define SN_WIRE_H_

#include "net/addr.h"
#include "io/naddr.h"

typedef struct {
    sn_net_addr_ser_t dst; /**< SecondNet destination address */
    sn_net_addr_ser_t src; /**< SecondNet source address(spoofable) */
    uint16_t ttl; /**< TTL(Time To Live), decreased when forwarded, when 0 message isnt forwarded any more */
    uint16_t len; /**< Length of the payload */
} sn_wire_net_header_t;

#endif/*SN_WIRE_H_*/
