/**
 * @file
 * Contains methods for managing entries(sn_net_addr + net_addr)
 * */

#ifndef SN_NET_ENTRY_H_
#define SN_NET_ENTRY_H_

#include "net/addr.h"
#include "io/naddr.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SN_NET_ENTRY_PRINTABLE_LEN (SN_NET_ADDR_PRINTABLE_LEN + 1 + SN_IO_NADDR_PRINTABLE_LEN)

typedef struct sn_net_entry_t_ sn_net_entry_t;

struct sn_net_entry_t_ {
	unsigned char is_set; /**< Is this entry set? */
    sn_net_addr_t addr; /**< SecondNet address */
    sn_io_naddr_t net_addr; /**< Traditional network address */
};

/**
 * Compares two addresses strcmp-style
 * @param A A
 * @param B B
 * @return strcmp-style result
 * */
int sn_net_entry_cmp(const sn_net_entry_t* A, const sn_net_entry_t* B);

/**
 * Same as sn_net_entry_cmp but with reversed order
 * @param A A
 * @param B B
 * @return Opposite of strcmp-style-result
 * */
int sn_net_entry_cmp_neg(const sn_net_entry_t* A, const sn_net_entry_t* B);

/**
 * Calculates the length of an entry array(like a leafset)
 * @param arr The entry array
 * @param max_len Maximum posible length
 * @return Length of the array. arr[len].is_set == 0
 * */
size_t sn_net_entry_array_len(const sn_net_entry_t arr[], size_t max_len);

/**
 * Calculates the closest entry to some address with level constrains
 * @param dst The target address
 * @param candidates An array of entry candidates
 * @param max Size of the array of candidates(some candidates can be unset)
 * @param self Optional. Used for level constraining.
 * @param min_level Requires self parameter. Minimum level of coincidence with self that the candidate should have.
 * @param[out] closest Return entry
 * */
void sn_net_entry_closest(const sn_net_addr_t* dst, const sn_net_entry_t candidates[], size_t max, const sn_net_addr_t* self, unsigned int min_level, sn_net_entry_t* closest);

/**
 * Creates an string representation of an entry
 * @param sne The entry
 * @param[out] out_str Pointer to buffer where string representation should be stored. Must have at least SN_NET_ENTRY_PRINTABLE_LEN size allocated.
 * @param sn_net_addr_precision If not 0 limits the sndnet address representation length. If 0 full address will be printed.
 * */
int sn_net_entry_to_str(const sn_net_entry_t* sne, char* out_str, size_t sn_net_addr_precision);

/**
 * Checks for strict equality
 * @param a A
 * @param b B
 * @return 1 if A==B, 0 otherwise
 * */
int sn_net_entry_equals(const sn_net_entry_t* a, const sn_net_entry_t* b);

/**
 * Serialized sn_net_entry_t
 * */
typedef struct sn_net_entry_ser_t_ {
	uint8_t is_set;
	sn_net_addr_ser_t addr;
	sn_io_naddr_ser_t net_addr;
} sn_net_entry_ser_t;

/**
 * Serializes a sn_net_entry
 * @param sne A sn_net_entry
 * @param ser Serialized sn_net_entry
 * @return 0 if OK, -1 if ERROR
 * */
int sn_net_entry_ser(const sn_net_entry_t* sne, sn_net_entry_ser_t* ser);

/**
 * Deserializes a serialized sn_net_entry
 * @param sne A sn_net_entry
 * @param ser Serialized sn_net_entry
 * @return 0 if OK, -1 if ERROR
 * */
int sn_net_entry_deser(sn_net_entry_t* sne, const sn_net_entry_ser_t* ser);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif/*SN_NET_ENTRY_H_*/
