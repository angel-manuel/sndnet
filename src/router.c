#include "router.h"

#include <assert.h>
#include <string.h>
#include <sys/socket.h>

void sndnet_router_set(SNRouter* snr, const SNEntry* sne);
int sndnet_router_get_leafset_size(const SNRouter* snr);
int sndnet_router_is_on_leafset_range(const SNRouter* snr, const SNAddress* addr);
void sndnet_router_closest(const SNAddress* dst, const SNEntry candidates[], size_t max, const SNAddress* self, unsigned int min_level, SNEntry* closest);

void sndnet_router_init(SNRouter* snr, const SNAddress* self) {
	assert(snr != 0);
	assert(self != 0);
	
	memset(snr, 0, sizeof(SNRouter));
	sndnet_address_copy(&(snr->self), self);
}

void sndnet_router_add(SNRouter* snr, const SNAddress* addr, const SNRealAddress* net_addr) {
	SNEntry e;

	assert(addr != 0);

	e.is_set = 1;
	sndnet_address_copy(&(e.sn_addr), addr);

	if(net_addr)
		memcpy(&(e.net_addr), net_addr, sizeof(SNRealAddress));
	else
		memset(&(e.net_addr), 0, sizeof(SNRealAddress));

	sndnet_router_set(snr, &e);
}

void sndnet_router_remove(SNRouter* snr, const SNAddress* addr) {
	SNEntry e;

	assert(addr != 0);

	e.is_set = 0;
	sndnet_address_copy(&(e.sn_addr), addr);

	sndnet_router_set(snr, &e);
}

void sndnet_router_nexthop(const SNRouter* snr, const SNAddress* dst, SNEntry* nexthop) {
	unsigned int level;
	unsigned char column;
	const SNEntry* e;
	SNEntry bests[4];
	
	assert(snr != 0);
	assert(dst != 0);
	assert(nexthop != 0);
	
	nexthop->is_set = 0;
	
	//Leafset routing
	
	if(sndnet_router_is_on_leafset_range(snr, dst)) {
		sndnet_router_closest(dst, snr->leafset, SNDNET_ROUTER_LEAFSET_SIZE, 0, 0, nexthop);
		return;
	}
	
	//Table routing
	
	sndnet_address_index(&(snr->self), dst, &level, &column);
	
	assert(column >= 0 && level < SNDNET_ROUTER_LEVELS);
	
	e = &(snr->table[level][column]);
	
	if(e->is_set) {
		memcpy(nexthop, e, sizeof(SNEntry));
		return;
	}
	
	//Best answer
	
	bests[3].is_set = 1;
	sndnet_address_copy(&(bests[3].sn_addr), &(snr->self));
	
	sndnet_router_closest(dst, snr->table[level], SNDNET_ROUTER_COLUMNS, 0, 0, &(bests[0]));
	sndnet_router_closest(dst, snr->leafset, SNDNET_ROUTER_LEAFSET_SIZE, &(snr->self), level, &(bests[1]));
	sndnet_router_closest(dst, snr->neighbourhood, SNDNET_ROUTER_NEIGHBOURHOOD_SIZE, &(snr->self), level, &(bests[2]));
	sndnet_router_closest(dst, bests, 4, 0, 0, nexthop);
	
	if(sndnet_address_cmp(&(nexthop->sn_addr), &(snr->self)) == 0) {
		nexthop->is_set = 0;
	}
}

int sndnet_router_get_leafset_size(const SNRouter* snr) {
	int count;
	
	assert(snr != NULL);
	
	if(!snr)
		return -1;
	
	for(count = 0; count < SNDNET_ROUTER_LEAFSET_SIZE; ++count) {
		if(snr->leafset[count].is_set == 0)
			break;
	}
	
	return count;
}

int sndnet_router_is_on_leafset_range(const SNRouter* snr, const SNAddress* addr) {
	int leaf_count;
	
	assert(snr != NULL);
	assert(addr != NULL);
	
	leaf_count = sndnet_router_get_leafset_size(snr);
	
	return leaf_count >= 2 &&
	sndnet_address_cmp(&(snr->leafset[0].sn_addr), addr) <= 0 &&
	sndnet_address_cmp(addr, &(snr->leafset[leaf_count-1].sn_addr)) <= 0;
}

void sndnet_router_closest(const SNAddress* dst, const SNEntry candidates[], size_t max, const SNAddress* self, unsigned int min_level, SNEntry* closest) {
	const SNEntry* e;
	const SNEntry* best = 0;
	SNAddress min_dist;
	SNAddress tmp_dist;
	int i;
	unsigned int level;
	
	assert(dst != 0);
	assert(candidates != 0);
	assert(max > 0);
	assert(closest != 0);
	
	for(i = 0; i < max; ++i) {
		e = &(candidates[i]);
		
		if(e->is_set) {
			if(self) { //min_level check needed
				sndnet_address_index(self, &(e->sn_addr), &level, 0);
				
				if(level < min_level)
					continue;
			}
			
			if(!best) {
				best = e;
				sndnet_address_dist(&(best->sn_addr), dst, &min_dist);
				continue;
			}
			
			sndnet_address_dist(&(e->sn_addr), dst, &tmp_dist);
			
			if(sndnet_address_cmp(&tmp_dist, &min_dist) < 0) {
				best = e;
				sndnet_address_copy(&min_dist, &tmp_dist);
			}
		}
	}
	
	if(best) {
		memcpy(closest, best, sizeof(SNEntry));
	} else {
		closest->is_set = 0;
	}
}

void sndnet_router_set(SNRouter* snr, const SNEntry* sne) {
	unsigned int level;
	unsigned char column;
	SNEntry* insert;
	const SNAddress* addr;
	
	assert(snr != 0);
	assert(sne != 0);
	
	addr = &(sne->sn_addr);
	
	sndnet_address_index(&(snr->self), addr, &level, &column);

	if(column >= 0) {
		insert = &(snr->table[level][column]);
		
		assert(sndnet_address_cmp(&(snr->self), &(insert->sn_addr)) != 0); //Should be impossible
		
		memcpy(insert, sne, sizeof(SNEntry));
	}
	
	//TODO: Add to leafset
}
