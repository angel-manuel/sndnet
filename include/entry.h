/**
 * @file
 * Contains methods for managing entries(sn_addr + net_addr)
 * */

#ifndef SN_ENTRY_H_
#define SN_ENTRY_H_

#include "addr.h"
#include "realaddr.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SN_ENTRY_PRINTABLE_LEN (SN_ADDR_PRINTABLE_LEN + 1 + SN_REALADDR_PRINTABLE_LEN)

typedef struct sn_entry_t_ sn_entry_t;

struct sn_entry_t_ {
	unsigned char is_set; /**< Is this entry set? */
    sn_addr_t sn_addr; /**< SecondNet address */
    sn_realaddr_t net_addr; /**< Traditional network address */
};

/**
 * Compares two addresses strcmp-style
 * @param A A
 * @param B B
 * @return strcmp-style result
 * */
int sn_entry_cmp(const sn_entry_t* A, const sn_entry_t* B);

/**
 * Same as sn_entry_cmp but with reversed order
 * @param A A
 * @param B B
 * @return Opposite of strcmp-style-result
 * */
int sn_entry_cmp_neg(const sn_entry_t* A, const sn_entry_t* B);

/**
 * Calculates the length of an entry array(like a leafset)
 * @param arr The entry array
 * @return Length of the array. arr[len].is_set == 0
 * */
size_t sn_entry_array_len(const sn_entry_t arr[]);

/**
 * Calculates the closest entry to some address with level constrains
 * @param dst The target address
 * @param candidates An array of entry candidates
 * @param max Size of the array of candidates(some candidates can be unset)
 * @param self Optional. Used for level constraining.
 * @param min_level Requires self parameter. Minimum level of coincidence with self that the candidate should have.
 * @param[out] closest Return entry
 * */
void sn_entry_closest(const sn_addr_t* dst, const sn_entry_t candidates[], size_t max, const sn_addr_t* self, unsigned int min_level, sn_entry_t* closest);

/**
 * Creates an string representation of an entry
 * @param sne The entry
 * @param[out] out_str Pointer to buffer where string representation should be stored. Must have at least SN_ENTRY_PRINTABLE_LEN size allocated.
 * @param sn_addr_precision If not 0 limits the sndnet address representation length. If 0 full address will be printed.
 * */
int sn_entry_tostr(const sn_entry_t* sne, char* out_str, size_t sn_addr_precision);

/**
 * Checks for strict equality
 * @param a A
 * @param b B
 * @return 1 if A==B, 0 otherwise
 * */
int sn_entry_equals(const sn_entry_t* a, const sn_entry_t* b);

/**
 * Serialized sn_entry_t
 * */
typedef struct sn_entry_ser_t_ {
	uint8_t is_set;
	sn_addr_ser_t sn_addr;
	sn_realaddr_ser_t net_addr;
} sn_entry_ser_t;

/**
 * Serializes a sn_entry
 * @param sne A sn_entry
 * @param ser Serialized sn_entry
 * @return 0 if OK, -1 if ERROR
 * */
int sn_entry_ser(const sn_entry_t* sne, sn_entry_ser_t* ser);

/**
 * Deserializes a serialized sn_entry
 * @param sne A sn_entry
 * @param ser Serialized sn_entry
 * @return 0 if OK, -1 if ERROR
 * */
int sn_entry_deser(sn_entry_t* sne, const sn_entry_ser_t* ser);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif/*SN_ENTRY_H_*/
