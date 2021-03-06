/**
 * @file
 * Provides routing logic
 * */

#ifndef SN_NET_ROUTER_H_
#define SN_NET_ROUTER_H_

#include "net/addr.h"
#include "net/entry.h"
#include "io/naddr.h"

#include <sys/socket.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Number of levels in the routing table */
#define SN_NET_ROUTER_LEVELS (SN_NET_ADDR_LEN*2)

/** Number of columns in the routing table */
#define SN_NET_ROUTER_COLUMNS 16

/** Size of the leafset. */
#define SN_NET_ROUTER_LEAFSET_SIZE 8

/**
 * Holds a routing table entry(sndnet addr + normal addr)
 */
typedef struct sn_net_entry_t_ sn_net_entry_t;

/**
 * Holds all the state relevant to routing.
 * Should NOT be used directly.
 * */
typedef struct sn_net_router_t_ sn_net_router_t;

/**
 * Initializes routing state
 * @param snr Router uninitialized state
 * @param self_addr Routing node address
 * @param self_net_addr Routing node network address
 * */
void sn_net_router_init(sn_net_router_t* snr, const sn_net_addr_t* self_addr, const sn_io_naddr_t* self_net_addr);

/**
 * Adds an entry to the routing info
 * @param snr Router state
 * @param addr Second Net address
 * @param net_addr Underlying network address
 * */
void sn_net_router_add(sn_net_router_t* snr, const sn_net_addr_t* addr, const sn_io_naddr_t* net_addr);

/**
 * Removes an entry from the routing info
 * @param snr Router state
 * @param addr Second Net address
 * */
void sn_net_router_remove(sn_net_router_t* snr, const sn_net_addr_t* addr);

/**
 * Tells the best nexthop
 * @param snr Router state
 * @param dst Destination address
 * @param[out] nexthop Entry to store the result
 * */
void sn_net_router_nexthop(const sn_net_router_t* snr, const sn_net_addr_t* dst, sn_net_entry_t* nexthop);

/**
 * Gets an string representation of the routing info
 * @param snr Router state
 * @param[out] out_str Pointer to string for placing the representation
 * @param out_str_len Allocated length of out_str
 * @return 0 if OK, -1 if ERROR
 * */
int sn_net_router_to_str(const sn_net_router_t* snr, char* out_str, size_t out_str_len);

/**
 * Returns a read-only pointer to a position on the routing table
 * @param snr Router state
 * @param level Table level
 * @param column Table column
 * @return A read-only pointer to the entry on the table
 * */
const sn_net_entry_t* sn_net_router_table_get(const sn_net_router_t* snr, unsigned int level, unsigned int column);

/**
 * Returns a read-only pointer to a position on the leafset
 * @param snr Router state
 * @param position Positive positions point to entries on the right leafset, negative one point to the left leafset and 0 gives router owner address.
 * @return A read-only pointer to the entry on the table
 * */
const sn_net_entry_t* sn_net_router_leafset_get(const sn_net_router_t* snr, int position);

/**
 * Copies an entry to a position on the routing table
 * @param snr Router state
 * @param level Table level
 * @param column Table column
 * @param e A pointer to the entry to be copied
 * */
void sn_net_router_table_set(sn_net_router_t* snr, unsigned int level, unsigned int column, const sn_net_entry_t* e);

/**
 * Copies an entry to a position on the leafset
 * @param snr Router state
 * @param position Positive positions point to entries on the right leafset, negative one point to the left leafset and 0 gives router owner address.
 * @param e A pointer to the entry to be copied
 * */
void sn_net_router_leafset_set(sn_net_router_t* snr, int position, const sn_net_entry_t* e);

typedef struct sn_net_router_entry_ser_t_ {
    uint8_t is_table;
    union {
        struct {
            uint16_t level;
            uint16_t column;
        };
        int32_t position;
    };
    sn_net_entry_ser_t entry;
} sn_net_router_entry_ser_t;

typedef struct sn_net_router_query_ser_t_ {
    uint32_t entries_len;
    sn_net_router_entry_ser_t entries[0];
} sn_net_router_query_ser_t;

/**
 * Queries entries from the table. Query result serialized and saved in *out_query. Should be freed.
 * @param snr Router state
 * @param l_min Minimum level.
 * @param l_max Maximum level.
 * @param[out] out_query *out_query will contain all the entries with level l l_min <= l <= l_max
 * @return 0 if ERROR; size of the query result otherwise
 * */
size_t sn_net_router_query_table(const sn_net_router_t* snr, uint16_t l_min, uint16_t l_max, sn_net_router_query_ser_t** out_query);

/**
 * Queries entries from the leafset. Query result serialized and saved in *out_query. Should be freed.
 * @param snr Router state
 * @param p_min Minimum position.
 * @param p_max Maximum position.
 * @param[out] out_query *out_query will contain all the entries with position p p_min <= p <= p_max
 * @return 0 if ERROR; size of the query result otherwise
 * */
size_t sn_net_router_query_leafset(const sn_net_router_t* snr, int32_t p_min, int32_t p_max, sn_net_router_query_ser_t** out_query);

struct sn_net_router_t_ {
    sn_net_entry_t self; /**< Our address */
    sn_net_entry_t table[SN_NET_ROUTER_LEVELS][SN_NET_ROUTER_COLUMNS]; /**< Routing table */
    sn_net_entry_t left_leafset[SN_NET_ROUTER_LEAFSET_SIZE]; /**< Leafset */
    sn_net_entry_t right_leafset[SN_NET_ROUTER_LEAFSET_SIZE]; /**< Leafset */
};

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif/*SN_NET_ROUTER_H_*/
