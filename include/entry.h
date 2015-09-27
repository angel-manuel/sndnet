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

int sndnet_entry_cmp(const sndnet_entry_t* A, const sndnet_entry_t* B);
int sndnet_entry_cmp_neg(const sndnet_entry_t* A, const sndnet_entry_t* B);
size_t sndnet_entry_array_len(const sndnet_entry_t* arr);
void sndnet_entry_closest(const sndnet_addr_t* dst, const sndnet_entry_t candidates[], size_t max, const sndnet_addr_t* self, unsigned int min_level, sndnet_entry_t* closest);
int sndnet_entry_tostr(const sndnet_entry_t* sna, char* out_str, size_t sn_addr_precision);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif/*SNDNET_ENTRY_H_*/