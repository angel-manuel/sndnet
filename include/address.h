#ifndef SNDNET_ADDRESS_H_
#define SNDNET_ADDRESS_H_

/**
 * SecondNet address length
 * */
#define SNDNET_ADDRESS_LENGTH 32

/**
 * Holds an address
 * */
typedef struct _SNAddress {
	unsigned char key[SNDNET_ADDRESS_LENGTH]; /**< Address key */
	unsigned char hex_key[SNDNET_ADDRESS_LENGTH*2]; /**< Precalculated hex representation of the key*/
} SNAddress;

/**
 * Initializes an address
 * @param snk SNAddress to be initialized
 * @param key Naked address key(the public key or some hash of it, TBD)
 * */
void sndnet_address_init(SNAddress* snk, unsigned char key[SNDNET_ADDRESS_LENGTH]);

/**
 * Gets the address key
 * @param sna The address
 * @return The address key
 * */
const unsigned char* sndnet_address_get(const SNAddress* sna);

/**
 * Gets the address key in hexadecimal
 * @param sna The address
 * @return The address key in hexadecimal
 * */
const unsigned char* sndnet_address_get_hex(const SNAddress* sna);

/**
 * Compares addresses(strcmp style)
 * @param a Some address
 * @param b Other address
 * @return negative if a < b, positive if a > b, zero if equal
 * */
int sndnet_address_cmp(const SNAddress* a, const SNAddress* b);

/**
 * Returns the absolute distance on the keyspace between two addresses
 * @param a Some address
 * @param b Other address
 * @param[out] The distance
 * */
void sndnet_address_dist(const SNAddress* a, const SNAddress* b, SNAddress* dist);

/**
 * Copies an address into another
 * @param[out] dst Destination
 * @param src Source
 * */
void sndnet_address_copy(SNAddress* dst, const SNAddress* src);

/**
 * Calculates the number of matching hex-chars from the beggining
 * between the two address and the first different hex of the second address.
 * Its useful for routing table lookup.
 * @param self Our address
 * @param addr The other address
 * @param[out] level The number of matching nibbles from the beggining
 * @param[out] column The first diferent nibble of the second address
 * */
void sndnet_address_index(const SNAddress* self, const SNAddress* addr, int* level, int* column);

#endif/*SNDNET_ADDRESS_H_*/
