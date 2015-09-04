#ifndef SNDNET_ADDRESS_H_
#define SNDNET_ADDRESS_H_

#define SNDNET_ADDRESS_LENGTH 32

typedef struct _SNAddress {
	unsigned char key[SNDNET_ADDRESS_LENGTH];
	unsigned char hex_key[SNDNET_ADDRESS_LENGTH*2];
} SNAddress;

void sndnet_address_init(SNAddress* snk, unsigned char key[SNDNET_ADDRESS_LENGTH]);
const unsigned char* sndnet_address_get(const SNAddress* snk);
const unsigned char* sndnet_address_get_hex(const SNAddress* snk);
int sndnet_address_cmp(const SNAddress* a, const SNAddress* b);
void sndnet_address_dist(const SNAddress* a, const SNAddress* b, SNAddress* dist);
void sndnet_address_copy(SNAddress* dst, const SNAddress* src);
void sndnet_address_index(const SNAddress* self, const SNAddress* _addr, int* level, int* column);

#endif/*SNDNET_ADDRESS_H_*/
