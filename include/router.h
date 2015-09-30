/**
 * @file
 * Provides routing logic
 * */

#ifndef SN_ROUTER_H_
#define SN_ROUTER_H_

#include "addr.h"
#include "entry.h"
#include "realaddr.h"

#include <sys/socket.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Number of levels in the routing table */
#define SN_ROUTER_LEVELS (SN_ADDR_LEN*2)

/** Number of columns in the routing table */
#define SN_ROUTER_COLUMNS 16

/** Size of the leafset. */
#define SN_ROUTER_LEAFSET_SIZE 8

/**
 * Holds a routing table entry(sndnet addr + normal addr)
 */
typedef struct sn_entry_t_ sn_entry_t;

/**
 * Holds all the state relevant to routing.
 * Should NOT be used directly.
 * */
typedef struct sn_router_t_ sn_router_t;

/**
 * Initializes routing state
 * @param snr Router uninitialized state
 * @param self Routing node address
 * */
void sn_router_init(sn_router_t* snr, const sn_addr_t* self);

/**
 * Adds an entry to the routing info
 * @param snr Router state
 * @param addr Second Net address
 * @param net_addr Underlying network address
 * */
void sn_router_add(sn_router_t* snr, const sn_addr_t* addr, const sn_realaddr_t* net_addr);

/**
 * Removes an entry from the routing info
 * @param snr Router state
 * @param addr Second Net address
 * */
void sn_router_remove(sn_router_t* snr, const sn_addr_t* addr);

/**
 * Tells the best nexthop
 * @param snr Router state
 * @param dst Destination address
 * @param[out] nexthop Entry to store the result
 * */
void sn_router_nexthop(const sn_router_t* snr, const sn_addr_t* dst, sn_entry_t* nexthop);

/**
 * Gets an string representation of the routing info
 * @param snr Router state
 * @param[out] out_str Pointer to string for placing the representation
 * @param out_str_len Allocated length of out_str
 * @return 0 if OK, -1 if ERROR
 * */
int sn_router_tostr(const sn_router_t* snr, char* out_str, size_t out_str_len);

struct sn_router_t_ {
    sn_addr_t self; /**< Our address */
    sn_entry_t table[SN_ROUTER_LEVELS][SN_ROUTER_COLUMNS]; /**< Routing table */
    sn_entry_t left_leafset[SN_ROUTER_LEAFSET_SIZE]; /**< Leafset */
    sn_entry_t right_leafset[SN_ROUTER_LEAFSET_SIZE]; /**< Leafset */
};

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif/*SN_ROUTER_H_*/
