/**
 * @file
 * Provides functions for managing SecondNet addresses
 * */

#ifndef SNDNET_ADDRESS_H_
#define SNDNET_ADDRESS_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * SecondNet address length
 * */
#define SNDNET_ADDRESS_LENGTH 32

#define SNDNET_ADDRESS_HEX_LENGTH (SNDNET_ADDRESS_LENGTH*2)

#define SNDNET_ADDRESS_PRINTABLE_LENGTH ((SNDNET_ADDRESS_LENGTH*2)+1)

/**
 * Holds an address
 * */
typedef struct sndnet_addr_t_ sndnet_addr_t;

/**
 * Initializes an address
 * @param snk sndnet_addr_t to be initialized
 * @param key Naked address key(the public key or some hash of it, TBD)
 * */
void sndnet_address_init(sndnet_addr_t* snk, const unsigned char key[SNDNET_ADDRESS_LENGTH]);

/**
 * Initializes an address from the hexadecimal string
 * @param snk sndnet_addr_t to be initialized
 * @param hexstr NULL-terminated strin with the hexadecimal representation of the key. If shorter than address length
 *               the rest is assumed to be "0x0"
 * */
void sndnet_address_from_hex(sndnet_addr_t* snk, const char* hexstr);

/**
 * Gets the address key
 * @param sna The address
 * @param[out] out_raw Pointer to place where save raw address. Must have at least SNDNET_ADDRESS_LENGTH size
 * */
void sndnet_address_get_raw(const sndnet_addr_t* sna, unsigned char* out_raw);

/**
 * Gets the address key in hexadecimal
 * @param sna The address
 * @param[out] out_hex Pointer to place where save hex address. Must have at least SNDNET_ADDRESS_HEX_LENGTH size
 * */
void sndnet_address_get_hex(const sndnet_addr_t* sna, unsigned char* out_hex);

/**
 * Returns a hex printable string
 * @param sna Address
 * @param[out] Printable hex address. Must have SNDNET_ADDRESS_PRINTABLE_LENGTH.
 * */
void sndnet_address_tostr(const sndnet_addr_t* sna, char* out_str);

/**
 * Compares addresses(strcmp style)
 * @param a Some address
 * @param b Other address
 * @return negative if a < b, positive if a > b, zero if equal
 * */
int sndnet_address_cmp(const sndnet_addr_t* a, const sndnet_addr_t* b);

/**
 * Returns the absolute distance on the keyspace between two addresses
 * @param a Some address
 * @param b Other address
 * @param[out] The distance
 * */
void sndnet_address_dist(const sndnet_addr_t* a, const sndnet_addr_t* b, sndnet_addr_t* dist);

/**
 * Calculates the number of matching hex-chars from the beggining
 * between the two address and the first different hex of the second address.
 * Its useful for routing table lookup.
 * @param self Our address
 * @param addr The other address
 * @param[out] level The number of matching nibbles from the beggining
 * @param[out] column The first diferent nibble of the second address
 * */
void sndnet_address_index(const sndnet_addr_t* self, const sndnet_addr_t* addr, unsigned int* level, unsigned char* column);

struct sndnet_addr_t_ {
    unsigned char key[SNDNET_ADDRESS_LENGTH]; /**< Address key */
};

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif/*SNDNET_ADDRESS_H_*/
