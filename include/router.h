/**
 * @file
 * Provides routing logic
 * */

#ifndef SNDNET_ROUTER_H_
#define SNDNET_ROUTER_H_

#include "address.h"

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

/** Size of the neighbourhood */
#define SNDNET_ROUTER_NEIGHBOURHOOD_SIZE 8

/**
 * Underlying network address
 * */
typedef struct sockaddr SNRealAddress;

/**
 * Holds a routing table entry(sndnet addr + normal addr)
 */
typedef struct SNEntry_ SNEntry;

/**
 * Holds all the state relevant to routing.
 * Should NOT be used directly.
 * */
typedef struct SNRouter_ SNRouter;

/**
 * Initializes routing state
 * @param snr Router uninitialized state
 * @param self Routing node address
 * */
void sndnet_router_init(SNRouter* snr, const SNAddress* self);

/**
 * Adds an entry to the routing info
 * @param snr Router state
 * @param addr Second Net address
 * @param net_addr Underlying network address
 * */
void sndnet_router_add(SNRouter* snr, const SNAddress* addr, const SNRealAddress* net_addr);

/**
 * Removes an entry from the routing info
 * @param snr Router state
 * @param addr Second Net address
 * */
void sndnet_router_remove(SNRouter* snr, const SNAddress* addr);

/**
 * Tells the best nexthop
 * @param snr Router state
 * @param dst Destination address
 * @param[out] nexthop Entry to store the result
 * */
void sndnet_router_nexthop(const SNRouter* snr, const SNAddress* dst, SNEntry* nexthop);

struct SNEntry_ {
    unsigned char is_set; /**< Is this entry set? */
    SNAddress sn_addr; /**< SecondNet address */
    SNRealAddress net_addr; /**< Traditional network address */
};

struct SNRouter_ {
    SNAddress self; /**< Our address */
    SNEntry table[SNDNET_ROUTER_LEVELS][SNDNET_ROUTER_COLUMNS]; /**< Routing table */
    SNEntry leafset[SNDNET_ROUTER_LEAFSET_SIZE]; /**< Leafset */
    SNEntry neighbourhood[SNDNET_ROUTER_NEIGHBOURHOOD_SIZE]; /**< Neighbourhood */
};

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif/*SNDNET_ROUTER_H_*/
