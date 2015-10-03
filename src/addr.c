#include "addr.h"

#include <assert.h>
#include <ctype.h>
#include <string.h>

void bytestring_to_hexstring(const unsigned char* bytestring, unsigned char* hexstring);
void bytestring_negate(const unsigned char* bytestring, unsigned char* negated);
void hexstring_to_printable(const unsigned char* hexstring, char* printable);
void printable_to_bytestring(const char* printable, unsigned char* bytestring);

void sn_addr_init(sn_addr_t* sna, const unsigned char key[SN_ADDR_LEN]) {
    assert(sna != 0);
    assert(key != 0);

    memcpy(sna->key, key, SN_ADDR_LEN);
}

void sn_addr_from_hex(sn_addr_t* sna, const char hexstr[SN_ADDR_HEX_LEN]) {
    unsigned char key[SN_ADDR_LEN];

    assert(hexstr != 0);

    printable_to_bytestring(hexstr, key);

    sn_addr_init(sna, key);
}

void sn_addr_get_raw(const sn_addr_t* sna, unsigned char* out_raw) {
    assert(sna != 0);
    assert(out_raw != 0);

    memcpy(out_raw, sna->key, SN_ADDR_LEN);
}

void sn_addr_get_hex(const sn_addr_t* sna, unsigned char* out_hex) {
    assert(sna != 0);
    assert(out_hex != 0);

    bytestring_to_hexstring(sna->key, out_hex);
}

void sn_addr_tostr(const sn_addr_t* sna, char* out_str) {
    unsigned char hexstr[SN_ADDR_HEX_LEN];

    assert(sna != 0);
    assert(out_str != 0);

    sn_addr_get_hex(sna, hexstr);
    hexstring_to_printable(hexstr, out_str);
}

int sn_addr_cmp(const sn_addr_t* a, const sn_addr_t* b) {
    int d, i;

    assert(a != 0);
    assert(b != 0);

    for(i = 0; i < SN_ADDR_LEN; ++i) {
        d = (int)a->key[i] - (int)b->key[i];

        if(d)
            return d;
    }

    return 0;
}

void sn_addr_dist(const sn_addr_t* a, const sn_addr_t* b, sn_addr_t* dist) {
    int i;
    unsigned char carry, ca, cb, cc;
    unsigned char sub[SN_ADDR_LEN];

    assert(a != 0);
    assert(b != 0);
    assert(dist != 0);

    // a - b

    carry = 0;
    for(i = (SN_ADDR_LEN - 1); i >= 0; --i) {
        ca = a->key[i];
        cb = b->key[i] + carry;

        carry = (cb > ca);
        cc = ca - cb;

        sub[i] = cc;
    }

    if(sub[0] >= 128) {
        bytestring_negate(sub, sub);
    }

    sn_addr_init(dist, sub);
}

void sn_addr_index(const sn_addr_t* self, const sn_addr_t* addr, unsigned int* level, unsigned char* column) {
    unsigned int l = 0;
    unsigned int i;
    unsigned char hex_a[SN_ADDR_HEX_LEN];
    unsigned char hex_b[SN_ADDR_HEX_LEN];

    assert(self != 0);
    assert(addr != 0);
    assert(level != 0 || column != 0);

    sn_addr_get_hex(self, hex_a);
    sn_addr_get_hex(addr, hex_b);

    if(column)
        *column = 255;

    for(i = 0; i < (SN_ADDR_LEN*2); ++i) {
        if(hex_a[i] == hex_b[i]) {
            ++l;
        } else {
            if(column)
                *column = hex_b[i];
            break;
        }
    }

    assert(l <= SN_ADDR_LEN*2);

    if(level)
        *level = l;
}

int sn_addr_ser(const sn_addr_t* sna, sn_addr_ser_t* ser) {
    assert(sna != 0);
    assert(ser != 0);

    memcpy(&ser->key, sna, SN_ADDR_LEN);

    return 0;
}

int sn_addr_deser(sn_addr_t* sna, const sn_addr_ser_t* ser) {
    assert(sna != 0);
    assert(ser != 0);

    memcpy(sna, &ser->key, SN_ADDR_LEN);

    return 0;
}

//Private

void bytestring_to_hexstring(const unsigned char* bytestring, unsigned char* hexstring) {
    int i;
    unsigned char c;

    assert(bytestring != 0);
    assert(hexstring != 0);

    for(i = 0; i < SN_ADDR_LEN; ++i) {
        c = bytestring[i];
        hexstring[2*i] = c/16;
        hexstring[(2*i)+1] = c%16;
    }
}

void bytestring_negate(const unsigned char* bytestring, unsigned char* negated) {
    int i;

    assert(bytestring != 0);
    assert(negated != 0);

    for(i = 0; i < SN_ADDR_LEN; ++i) {
        negated[i] = 255 ^ bytestring[i];
    }
}

void hexstring_to_printable(const unsigned char* hexstring, char* printable) {
    int i;
    char c;

    assert(hexstring != 0);
    assert(printable != 0);

    for(i = 0; i < SN_ADDR_LEN*2; ++i) {
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
    int c;
    unsigned char ac = 0;

    memset(bytestring, 0, SN_ADDR_LEN);

    for(i = 0; printable[i] != '\0' && i < SN_ADDR_LEN*2; ++i) {
        c = (int)printable[i];

        assert(isalnum(c));

        if(isdigit(c)) {
            ac += (c - '0');
        } else if(islower(c)) {
            ac += (c - 'a' + 10);
        } else if(isupper(c)) {
            ac += (c - 'A' + 10);
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
