/**
 * @file
 * Contains the main logic
 * */

#ifndef SN_CORE_H_
#define SN_CORE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "sndnet.h"
#include "net/entry.h"
#include "net/packet.h"

int sn_core_deliver(sn_state_t* state, const sn_net_packet_t* packet, const sn_io_naddr_t* rem_addr);
int sn_core_forward(sn_state_t* state, sn_net_packet_t* packet, const sn_io_naddr_t* rem_addr, sn_net_entry_t* nexthop);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif/*SN_CORE_H_*/
