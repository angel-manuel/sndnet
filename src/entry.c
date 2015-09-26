#include "entry.h"

#include "address.h"

#include <assert.h>

int sndnet_entry_cmp(const sndnet_entry_t* A, const sndnet_entry_t* B) {
	return sndnet_address_cmp(&A->sn_addr, &B->sn_addr);
}

int sndnet_entry_cmp_neg(const sndnet_entry_t* A, const sndnet_entry_t* B) {
	return sndnet_entry_cmp(B, A);
}

size_t sndnet_entry_array_len(const sndnet_entry_t* arr) {
	size_t ret = 0;
	
	assert(arr != 0);
	
	while(arr[ret].is_set)
		++ret;
	
	return ret;
}

void sndnet_entry_closest(const sndnet_addr_t* dst, const sndnet_entry_t candidates[], size_t max, const sndnet_addr_t* self, unsigned int min_level, sndnet_entry_t* closest) {
    const sndnet_entry_t* best = 0;
    sndnet_addr_t min_dist;
    sndnet_addr_t tmp_dist;
    unsigned int i;
    unsigned int level;
    
    assert(dst != 0);
    assert(candidates != 0);
    assert(max > 0);
    assert(closest != 0);
    
    for(i = 0; i < max; ++i) {
        const sndnet_entry_t* e = &candidates[i];
        
        if(e->is_set) {
            if(self) {
                sndnet_address_index(self, &e->sn_addr, &level, 0);
                
                if(level < min_level)
                    continue;
            }
            
            if(!best) {
                best = e;
                sndnet_address_dist(&best->sn_addr, dst, &min_dist);
                continue;
            }
            
            sndnet_address_dist(&e->sn_addr, dst, &tmp_dist);
            
            if(sndnet_address_cmp(&tmp_dist, &min_dist) < 0) {
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