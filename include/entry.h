#ifndef SNDNET_ENTRY_H_
#define SNDNET_ENTRY_H_

#include "address.h"
#include "realaddress.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SNDNET_ENTRY_PRINTABLE_LENGTH (SNDNET_ADDRESS_PRINTABLE_LENGTH + 1 + SNDNET_REALADDRESS_PRINTABLE_LENGTH)

typedef struct sndnet_entry_t_ sndnet_entry_t;

struct sndnet_entry_t_ {
	unsigned char is_set; /**< Is this entry set? */
    sndnet_addr_t sn_addr; /**< SecondNet address */
    sndnet_realaddr_t net_addr; /**< Traditional network address */
};

/**
 * Compares two addresses strcmp-style
 * @param A A
 * @param B B
 * @return strcmp-style result
 * */
int sndnet_entry_cmp(const sndnet_entry_t* A, const sndnet_entry_t* B);

/**
 * Same as sndnet_entry_cmp but with reversed order
 * @param A A
 * @param B B
 * @return Opposite of strcmp-style-result
 * */
int sndnet_entry_cmp_neg(const sndnet_entry_t* A, const sndnet_entry_t* B);

/**
 * Calculates the length of an entry array(like a leafset)
 * @param arr The entry array
 * @return Length of the array. arr[len].is_set == 0
 * */
size_t sndnet_entry_array_len(const sndnet_entry_t arr[]);

/**
 * Calculates the closest entry to some address with level constrains
 * @param dst The target address
 * @param candidates An array of entry candidates
 * @param max Size of the array of candidates(some candidates can be unset)
 * @param self Optional. Used for level constraining.
 * @param min_level Requires self parameter. Minimum level of coincidence with self that the candidate should have.
 * @param[out] closest Return entry
 * */
void sndnet_entry_closest(const sndnet_addr_t* dst, const sndnet_entry_t candidates[], size_t max, const sndnet_addr_t* self, unsigned int min_level, sndnet_entry_t* closest);

/**
 * Creates an string representation of an entry
 * @param sne The entry
 * @param[out] out_str Pointer to buffer where string representation should be stored. Must have at least SNDNET_ENTRY_PRINTABLE_LENGTH size allocated.
 * @param sn_addr_precision If not 0 limits the sndnet address representation length. If 0 full address will be printed.
 * */
int sndnet_entry_tostr(const sndnet_entry_t* sne, char* out_str, size_t sn_addr_precision);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif/*SNDNET_ENTRY_H_*/