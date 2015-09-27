/**
 * @file
 * Provides routing logic
 * */

#ifndef SNDNET_ROUTER_H_
#define SNDNET_ROUTER_H_

#include "address.h"
#include "entry.h"
#include "realaddress.h"

#include <sys/socket.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Number of levels in the routing table */
#define SNDNET_ROUTER_LEVELS (SNDNET_ADDRESS_LENGTH*2)

/** Number of columns in the routing table */
#define SNDNET_ROUTER_COLUMNS 16

/** Size of the leafset. */
#define SNDNET_ROUTER_LEAFSET_SIZE 8

/**
 * Holds a routing table entry(sndnet addr + normal addr)
 */
typedef struct sndnet_entry_t_ sndnet_entry_t;

/**
 * Holds all the state relevant to routing.
 * Should NOT be used directly.
 * */
typedef struct sndnet_router_t_ sndnet_router_t;

/**
 * Initializes routing state
 * @param snr Router uninitialized state
 * @param self Routing node address
 * */
void sndnet_router_init(sndnet_router_t* snr, const sndnet_addr_t* self);

/**
 * Adds an entry to the routing info
 * @param snr Router state
 * @param addr Second Net address
 * @param net_addr Underlying network address
 * */
void sndnet_router_add(sndnet_router_t* snr, const sndnet_addr_t* addr, const sndnet_realaddr_t* net_addr);

/**
 * Removes an entry from the routing info
 * @param snr Router state
 * @param addr Second Net address
 * */
void sndnet_router_remove(sndnet_router_t* snr, const sndnet_addr_t* addr);

/**
 * Tells the best nexthop
 * @param snr Router state
 * @param dst Destination address
 * @param[out] nexthop Entry to store the result
 * */
void sndnet_router_nexthop(const sndnet_router_t* snr, const sndnet_addr_t* dst, sndnet_entry_t* nexthop);

int sndnet_router_tostr(const sndnet_router_t* snr, char* out_str, size_t out_str_len);

struct sndnet_router_t_ {
    sndnet_addr_t self; /**< Our address */
    sndnet_entry_t table[SNDNET_ROUTER_LEVELS][SNDNET_ROUTER_COLUMNS]; /**< Routing table */
    sndnet_entry_t left_leafset[SNDNET_ROUTER_LEAFSET_SIZE]; /**< Leafset */
    sndnet_entry_t right_leafset[SNDNET_ROUTER_LEAFSET_SIZE]; /**< Leafset */
};

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif/*SNDNET_ROUTER_H_*/
