/**
 * @file
 * Provides functions for managing SecondNet addresses
 * */

#ifndef SN_ADDR_H_
#define SN_ADDR_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * SecondNet address length
 * */
#define SN_ADDR_LEN 32

/**
 * SecondNet address hexadecimal length.
 * */
#define SN_ADDR_HEX_LEN (SN_ADDR_LEN*2)

/**
 * SecondNet address printable representation length.
 * */
#define SN_ADDR_PRINTABLE_LEN ((SN_ADDR_LEN*2)+1)

/**
 * Holds an address
 * */
typedef struct sn_addr_t_ sn_addr_t;

/**
 * Initializes an address
 * @param sna sn_addr_t to be initialized
 * @param key Naked address key(the public key or some hash of it, TBD)
 * */
void sn_addr_init(sn_addr_t* sna, const unsigned char key[SN_ADDR_LEN]);

/**
 * Initializes an address from the hexadecimal string
 * @param sna sn_addr_t to be initialized
 * @param hexstr NULL-terminated strin with the hexadecimal representation of the key. If shorter than address length
 *               the rest is assumed to be "0x0"
 * */
void sn_addr_from_hex(sn_addr_t* sna, const char hexstr[SN_ADDR_HEX_LEN]);

/**
 * Gets the address key
 * @param sna The address
 * @param[out] out_raw Pointer to place where save raw address. Must have at least SN_ADDR_LEN size
 * */
void sn_addr_get_raw(const sn_addr_t* sna, unsigned char* out_raw);

/**
 * Gets the address key in hexadecimal
 * @param sna The address
 * @param[out] out_hex Pointer to place where save hex address. Must have at least SN_ADDR_HEX_LEN size
 * */
void sn_addr_get_hex(const sn_addr_t* sna, unsigned char* out_hex);

/**
 * Returns a hex printable string
 * @param sna Address
 * @param[out] out_str Printable hex address. Must have SN_ADDR_PRINTABLE_LEN.
 * */
void sn_addr_tostr(const sn_addr_t* sna, char* out_str);

/**
 * Compares addresses(strcmp style)
 * @param a Some address
 * @param b Other address
 * @return negative if a < b, positive if a > b, zero if equal
 * */
int sn_addr_cmp(const sn_addr_t* a, const sn_addr_t* b);

/**
 * Returns the absolute distance on the keyspace between two addresses
 * @param a Some address
 * @param b Other address
 * @param[out] dist The distance
 * */
void sn_addr_dist(const sn_addr_t* a, const sn_addr_t* b, sn_addr_t* dist);

/**
 * Calculates the number of matching hex-chars from the beggining
 * between the two address and the first different hex of the second address.
 * Its useful for routing table lookup.
 * @param self Our address
 * @param addr The other address
 * @param[out] level The number of matching nibbles from the beggining
 * @param[out] column The first diferent nibble of the second address
 * */
void sn_addr_index(const sn_addr_t* self, const sn_addr_t* addr, unsigned int* level, unsigned char* column);

/**
 * Serialized sn_addr
 * */
typedef struct sn_addr_ser_t_ {
    unsigned char key[SN_ADDR_LEN]; /**< Address key */
} sn_addr_ser_t;

/**
 * Serializes a sn_addr
 * @param sna A sn_addr
 * @param ser Pointer to serialized sn_addr
 * @return 0 if OK, -1 if ERROR
 * */
int sn_addr_ser(const sn_addr_t* sna, sn_addr_ser_t* ser);

/**
 * Deserializes a serialized sn_addr
 * @param sna Deserialized sn_addr
 * @param ser Pointer to serialized sn_addr
 * @return 0 if OK, -1 if ERROR
 * */
int sn_addr_deser(sn_addr_t* sna, const sn_addr_ser_t* ser);

struct sn_addr_t_ {
    unsigned char key[SN_ADDR_LEN]; /**< Address key */
};

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif/*SN_ADDR_H_*/
