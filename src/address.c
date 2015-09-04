#include "address.h"

#include <assert.h>
#include <string.h>

void bytestring_to_hexstring(const unsigned char* bytestring, int n, unsigned char* hexstring);
void bytestring_negate(const unsigned char* bytestring, int n, unsigned char* negated);

void sndnet_address_init(SNAddress* snk, unsigned char key[SNDNET_ADDRESS_LENGTH]) {
	assert(snk != 0);
	
	memcpy(snk->key, key, SNDNET_ADDRESS_LENGTH);
	bytestring_to_hexstring(snk->key, SNDNET_ADDRESS_LENGTH, snk->hex_key);
}

const unsigned char* sndnet_address_get(const SNAddress* snk) {
	assert(snk != 0);
	return snk->key;
}

const unsigned char* sndnet_address_get_hex(const SNAddress* snk) {
	assert(snk != 0);
	return snk->hex_key;
}

int sndnet_address_cmp(const SNAddress* a, const SNAddress* b) {
	assert(a != 0);
	assert(b != 0);
	
	return strncmp((char*)a->key, (char*)b->key, SNDNET_ADDRESS_LENGTH);
}

void sndnet_address_dist(const SNAddress* a, const SNAddress* b, SNAddress* dist) {
	int i;
	int cmp;
	const SNAddress* swap;
	unsigned char carry, ca, cb, cc;
	unsigned char sub[SNDNET_ADDRESS_LENGTH];
	
	assert(a != 0);
	assert(b != 0);
	assert(dist != 0);
	
	// a - b
	
	carry = 0;
	for(i = (SNDNET_ADDRESS_LENGTH - 1); i >= 0; --i) {
		ca = a->key[i] - carry;
		cb = b->key[i];
		
		carry = (ca >= cb);
		cc = ca - cb;
		
		sub[i] = cc;
	}
	
	if(sub[0] >= 128) {
		bytestring_negate(sub, SNDNET_ADDRESS_LENGTH, sub);
	}
	
	sndnet_address_init(dist, sub);
}

void sndnet_address_copy(SNAddress* dst, const SNAddress* src) {
	assert(dst != 0);
	assert(src != 0);
	
	memcpy(dst, src, sizeof(SNAddress));
}

void sndnet_address_index(const SNAddress* self, const SNAddress* _addr, int* level, int* column) {
	int l = 0;
	int i;
	const unsigned char *hex_a, *hex_b;
	
	assert(self != 0);
	assert(_addr != 0);
	assert(level != 0);
	assert(column != 0);
	
	hex_a = sndnet_address_get_hex(self);
	hex_b = sndnet_address_get_hex(_addr);
	
	*column = -1;
	
	for(i = 0; i < (SNDNET_ADDRESS_LENGTH*2); ++i) {
		if(hex_a[i] == hex_b[i]) {
			++l;
		} else {
			*column = (int)hex_b[i];
			break;
		}
	}
	
	assert(*column != -1);
	assert(level >= 0);
	
	*level = l;
}

//Private

void bytestring_to_hexstring(const unsigned char* bytestring, int n, unsigned char* hexstring) {
	int i;
	unsigned char c;
	
	assert(bytestring != 0);
	assert(hexstring != 0);
	
	for(i = 0; i < n; ++i) {
		c = bytestring[i];
		hexstring[2*i] = c/16;
		hexstring[(2*i)+1] = c%16;
	}
}

void bytestring_negate(const unsigned char* bytestring, int n, unsigned char* negated) {
	int i;
	
	assert(bytestring != 0);
	
	for(i = 0; i < n; ++i) {
		negated[i] = 255 ^ bytestring[i];
	}
}
