#ifndef SN_HANDLER_H_
#define SN_HANDLER_H_

#include "node.h"
#include "net/entry.h"
#include "net/packet.h"
#include "io/naddr.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*sn_forward_handler_t)(sn_node_t* sns, sn_net_packet_t* packet, const sn_io_naddr_t* rem_addr, sn_net_entry_t* nexthop);
typedef int (*sn_deliver_handler_t)(sn_node_t* sns, const sn_net_packet_t* packet, const sn_io_naddr_t* rem_addr);

extern const sn_forward_handler_t sn_default_forward_handlers[];
extern const sn_deliver_handler_t sn_default_deliver_handlers[];

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif/*SN_HANDLER_H_*/
