#include "router.h"

#include <assert.h>
#include <string.h>
#include <sys/socket.h>

void sndnet_router_init(SNRouter* snr, const SNAddress* self) {
	assert(snr != 0);
	assert(self != 0);
	
	memset(snr, 0, sizeof(SNRouter));
	sndnet_address_copy(&(snr->self), self);
}

void sndnet_router_add(SNRouter* snr, const SNEntry* sne) {
	int level, column;
	SNEntry* insert;
	
	assert(snr != 0);
	assert(sne != 0);
	
	sndnet_address_index(&(snr->self), &(sne->sn_addr), &level, &column);

	if(column >= 0) {
		insert = &(snr->table[level][column]);
		
		assert(sndnet_address_cmp(&(snr->self), &(insert->sn_addr)) != 0); //Should be impossible
		
		memcpy(insert, sne, sizeof(SNEntry));
	}
	
	//TODO: Add to leafset
}

void sndnet_router_nexthop(const SNRouter* snr, const SNAddress* dst, SNEntry* nexthop) {
	int level, column, i;
	const SNEntry* e;
	const SNEntry* best;
	SNEntry eself;
	SNAddress min_dist;
	SNAddress tmp_dist;
	
	assert(snr != 0);
	assert(dst != 0);
	assert(nexthop != 0);
	
	nexthop->is_set = 0;
	
	//TODO: Check leafset
	
	//Table routing
	
	sndnet_address_index(&(snr->self), dst, &level, &column);
	
	assert(column >= 0 && level < SNDNET_ROUTER_LEVELS);
	
	e = &(snr->table[level][column]);
	
	if(e->is_set) {
		memcpy(nexthop, e, sizeof(SNEntry));
		return;
	}
	
	//Best answer
	
	//Default best answer: self
	eself.is_set = 1;
	memcpy(&(eself.sn_addr), &(snr->self), sizeof(SNAddress));
	memset(&(eself.net_addr), 0, sizeof(eself.net_addr));
	best = &eself;
	
	sndnet_address_dist(&(eself.sn_addr), dst, &min_dist);
	
	for(i = 0; i < SNDNET_ROUTER_LEAFSET; ++i) {
		e = &(snr->leafset[i]);
		
		if(e->is_set) {
			sndnet_address_dist(&(e->sn_addr), dst, &tmp_dist);
			
			if(sndnet_address_cmp(&tmp_dist, &min_dist) < 0) {
				min_dist = tmp_dist;
				best = e;
			}
		}
	}
	
	for(i = 0; i < SNDNET_ROUTER_COLUMNS; ++i) {
		e = &(snr->table[level][i]);
		
		if(e->is_set) {
			sndnet_address_dist(&(e->sn_addr), dst, &tmp_dist);
			
			if(sndnet_address_cmp(&tmp_dist, &min_dist) < 0) {
				min_dist = tmp_dist;
				best = e;
			}
		}
	}
	
	for(i = 0; i < SNDNET_ROUTER_NEIGHBOURHOOD; ++i) {
		e = &(snr->neighbourhood[i]);
		
		if(e->is_set) {
			sndnet_address_dist(&(e->sn_addr), dst, &tmp_dist);
			
			if(sndnet_address_cmp(&tmp_dist, &min_dist) < 0) {
				min_dist = tmp_dist;
				best = e;
			}
		}
	}
	
	memcpy(nexthop, best, sizeof(SNEntry));
}
