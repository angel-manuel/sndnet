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

#define SNDNET_ADDRESS_PRINTABLE_LENGTH ((SNDNET_ADDRESS_LENGTH*2)+1)

/**
 * Holds an address
 * */
typedef struct sndnet_addr_t_ sndnet_addr_t;

/**
 * Underlying network address
 * */
typedef struct sockaddr sndnet_realaddr_t;

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
void sndnet_address_from_hexstr(sndnet_addr_t* snk, const char* hexstr);

/**
 * Gets the address key
 * @param sna The address
 * @return The address key
 * */
const unsigned char* sndnet_address_get(const sndnet_addr_t* sna);

/**
 * Gets the address key in hexadecimal
 * @param sna The address
 * @return The address key in hexadecimal
 * */
const unsigned char* sndnet_address_get_hex(const sndnet_addr_t* sna);

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
 * Copies an address into another
 * @param[out] dst Destination
 * @param src Source
 * */
void sndnet_address_copy(sndnet_addr_t* dst, const sndnet_addr_t* src);

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

/**
 * Returns a hex printable string
 * @param sna Address
 * @return Printable hex address
 * */
const char* sndnet_address_tostr(const sndnet_addr_t* sna);

struct sndnet_addr_t_ {
    unsigned char key[SNDNET_ADDRESS_LENGTH]; /**< Address key */
    unsigned char hex_key[SNDNET_ADDRESS_LENGTH*2]; /**< Precalculated hex representation of the key*/
    char printable[SNDNET_ADDRESS_LENGTH*2 + 1];
};

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif/*SNDNET_ADDRESS_H_*/
