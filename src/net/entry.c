#include "net/entry.h"

#include "net/addr.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

int sn_net_entry_cmp(const sn_net_entry_t* A, const sn_net_entry_t* B) {
	return sn_net_addr_cmp(&A->addr, &B->addr);
}

int sn_net_entry_cmp_neg(const sn_net_entry_t* A, const sn_net_entry_t* B) {
	return sn_net_entry_cmp(B, A);
}

size_t sn_net_entry_array_len(const sn_net_entry_t arr[], size_t max_len) {
	size_t ret = 0;

	assert(arr != 0);

	while(ret < max_len && arr[ret].is_set)
		++ret;

	return ret;
}

void sn_net_entry_closest(const sn_net_addr_t* dst, const sn_net_entry_t candidates[], size_t max, const sn_net_addr_t* self, unsigned int min_level, sn_net_entry_t* closest) {
    const sn_net_entry_t* best = 0;
    sn_net_addr_t min_dist;
    sn_net_addr_t tmp_dist;
    unsigned int i;
    unsigned int level;

    assert(dst != 0);
    assert(candidates != 0);
    assert(max > 0);
    assert(closest != 0);

    for(i = 0; i < max; ++i) {
        const sn_net_entry_t* e = &candidates[i];

        if(e->is_set) {
            if(self) {
                sn_net_addr_index(self, &e->addr, &level, 0);

                if(level < min_level)
                    continue;
            }

            if(!best) {
                best = e;
                sn_net_addr_dist(&best->addr, dst, &min_dist);
                continue;
            }

            sn_net_addr_dist(&e->addr, dst, &tmp_dist);

            if(sn_net_addr_cmp(&tmp_dist, &min_dist) < 0) {
                best = e;
				min_dist = tmp_dist;
            }
        }
    }

    if(best) {
		*closest = *best;
    } else {
        closest->is_set = 0;
    }
}

int sn_net_entry_to_str(const sn_net_entry_t* sne, char* out_str, size_t addr_precision) {
    char addr_str[SN_NET_ADDR_PRINTABLE_LEN];
    char net_addr_str[SN_IO_NADDR_PRINTABLE_LEN];

    assert(sne != 0);
    assert(out_str != 0);
    assert(addr_precision < SN_NET_ADDR_PRINTABLE_LEN);

    if(sne->is_set == 0) {
        if(snprintf(out_str, SN_NET_ENTRY_PRINTABLE_LEN, "NULL") < 4)
            return -1;

        return 0;
    }

    sn_net_addr_to_str(&sne->addr, addr_str);
    if(addr_precision)
        addr_str[addr_precision] = '\0';

    if(sn_io_naddr_to_str(&sne->net_addr, net_addr_str))
        return -1;


    if(snprintf(out_str, SN_NET_ENTRY_PRINTABLE_LEN, "%s@%s", addr_str, net_addr_str)
        < (ssize_t)(strlen(addr_str) + strlen(net_addr_str) + 1))
        return -1;

    return 0;
}

int sn_net_entry_equals(const sn_net_entry_t* a, const sn_net_entry_t* b) {
	assert(a != 0);
	assert(b != 0);

	if(a->is_set != b->is_set)
		return 0;

	if(sn_net_addr_cmp(&a->addr, &b->addr) != 0)
		return 0;

	/*if(sn_netaddr_cmp(&a->net_addr, &b->net_addr) != 0)
		return 0;*/

	return 1;
}

int sn_net_entry_ser(const sn_net_entry_t* sne, sn_net_entry_ser_t* ser) {
	assert(sne != 0);
	assert(ser != 0);

	ser->is_set = (uint8_t)sne->is_set;

	if(sn_net_addr_ser(&sne->addr, &ser->addr) < 0)
		return -1;

	if(sn_io_naddr_ser(&sne->net_addr, &ser->net_addr) < 0)
		return -1;

	return 0;
}

int sn_net_entry_deser(sn_net_entry_t* sne, const sn_net_entry_ser_t* ser) {
	assert(sne != 0);
	assert(ser != 0);

	sne->is_set = (unsigned char)ser->is_set;

	if(sn_net_addr_deser(&sne->addr, &ser->addr) < 0)
		return -1;

	if(sn_io_naddr_deser(&sne->net_addr, &ser->net_addr) < 0)
		return -1;

	return 0;
}
