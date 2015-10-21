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

/**
 * Delivers a packet calling all the logic
 * @param state Node state
 * @param packet packet
 * @param rem_addr Address from with the packet comes from or NULL if sent from local
 * @return 0 if OK, -1 if ERR
 * */
int sn_core_deliver(sn_node_t* state, const sn_net_packet_t* packet, const sn_io_naddr_t* rem_addr);

/**
 * Forwards a packet calling all the logic
 * @param state Node state
 * @param packet packet
 * @param rem_addr Address from with the packet comes from or NULL if sent from local
 * @param nexthop Router best next hop. Can be modified by the routine and will contain the final next hop.
 * @return 0 if OK, -1 if ERR
 * */
int sn_core_forward(sn_node_t* state, sn_net_packet_t* packet, const sn_io_naddr_t* rem_addr, sn_net_entry_t* nexthop);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif/*SN_CORE_H_*/
