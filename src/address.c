#include "address.h"

#include <assert.h>
#include <string.h>

void bytestring_to_hexstring(const unsigned char* bytestring, unsigned char* hexstring);
void bytestring_negate(const unsigned char* bytestring, unsigned char* negated);
void hexstring_to_printable(const unsigned char* hexstring, char* printable);
void printable_to_bytestring(const char* printable, unsigned char* bytestring);

void sndnet_address_init(SNAddress* snk, unsigned char key[SNDNET_ADDRESS_LENGTH]) {
	assert(snk != 0);
	
	memcpy(snk->key, key, SNDNET_ADDRESS_LENGTH);
	bytestring_to_hexstring(snk->key, snk->hex_key);
	hexstring_to_printable(snk->hex_key, snk->printable);
}

void sndnet_address_from_hexstr(SNAddress* snk, const char* hexstr) {
	unsigned char key[SNDNET_ADDRESS_LENGTH];
	
	assert(hexstr != 0);
	
	printable_to_bytestring(hexstr, key);
	
	sndnet_address_init(snk, key);
}

const unsigned char* sndnet_address_get(const SNAddress* sna) {
	assert(sna != 0);
	return sna->key;
}

const unsigned char* sndnet_address_get_hex(const SNAddress* sna) {
	assert(sna != 0);
	return sna->hex_key;
}

int sndnet_address_cmp(const SNAddress* a, const SNAddress* b) {
	assert(a != 0);
	assert(b != 0);
	
	return strncmp((char*)a->key, (char*)b->key, SNDNET_ADDRESS_LENGTH);
}

void sndnet_address_dist(const SNAddress* a, const SNAddress* b, SNAddress* dist) {
	int i;
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
		bytestring_negate(sub, sub);
	}
	
	sndnet_address_init(dist, sub);
}

void sndnet_address_copy(SNAddress* dst, const SNAddress* src) {
	assert(dst != 0);
	assert(src != 0);
	
	memcpy(dst, src, sizeof(SNAddress));
}

void sndnet_address_index(const SNAddress* self, const SNAddress* addr, unsigned int* level, unsigned char* column) {
	unsigned int l = 0;
	unsigned int i;
	const unsigned char *hex_a, *hex_b;
	
	assert(self != 0);
	assert(addr != 0);
	assert(level != 0 || column != 0);
	
	hex_a = sndnet_address_get_hex(self);
	hex_b = sndnet_address_get_hex(addr);
	
	if(column)
		*column = 255;
	
	for(i = 0; i < (SNDNET_ADDRESS_LENGTH*2); ++i) {
		if(hex_a[i] == hex_b[i]) {
			++l;
		} else {
			if(column)
				*column = (int)hex_b[i];
			break;
		}
	}
	
	if(column)
		assert(*column != 255 || l < SNDNET_ADDRESS_LENGTH*2);
		
	assert(l >= 0);
	
	if(level)
		*level = l;
}

const char* sndnet_address_tostr(const SNAddress* sna) {
	assert(sna != 0);
	
	return sna->printable;
}

//Private

void bytestring_to_hexstring(const unsigned char* bytestring, unsigned char* hexstring) {
	int i;
	unsigned char c;
	
	assert(bytestring != 0);
	assert(hexstring != 0);
	
	for(i = 0; i < SNDNET_ADDRESS_LENGTH; ++i) {
		c = bytestring[i];
		hexstring[2*i] = c/16;
		hexstring[(2*i)+1] = c%16;
	}
}

void bytestring_negate(const unsigned char* bytestring, unsigned char* negated) {
	int i;
	
	assert(bytestring != 0);
	assert(negated != 0);
	
	for(i = 0; i < SNDNET_ADDRESS_LENGTH; ++i) {
		negated[i] = 255 ^ bytestring[i];
	}
}

void hexstring_to_printable(const unsigned char* hexstring, char* printable) {
	int i;
	char c;
	
	assert(hexstring != 0);
	assert(printable != 0);
	
	for(i = 0; i < SNDNET_ADDRESS_LENGTH*2; ++i) {
		c = (char)hexstring[i];
		
		assert(c >= 0 && c < 16);
		
		if(c >= 0 && c < 10) {
			printable[i] = '0' + c;
		} else {
			printable[i] = 'W' + c;
		}
	}
	
	printable[i] = '\0';
}

void printable_to_bytestring(const char* printable, unsigned char* bytestring) {
	int i;
	char c;
	unsigned char ac = 0;
	
	memset(bytestring, 0, SNDNET_ADDRESS_LENGTH);
	
	for(i = 0; printable[i] != '\0' && i < SNDNET_ADDRESS_LENGTH*2; ++i) {
		c = printable[i];
		
		if(c >= '0' && c <= '9') {
			ac += (c - '0');
		} else {
			ac += (c - 'W');
		}
		
		if(i % 2) {
			bytestring[i/2] = ac;
			ac = 0;
		} else {
			ac = ac << 4;
		}
	}
	
	if(i % 2) {
		bytestring[i/2] = ac;
	}
}
