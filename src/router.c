#include "router.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

void sndnet_router_set(sndnet_router_t* snr, const sndnet_entry_t* sne);
void leafset_add(sndnet_router_t* snr, const sndnet_entry_t* sne);
void leafset_remove(sndnet_router_t* snr, const sndnet_entry_t* sne);
void leafset_insert(sndnet_entry_t* leafset, const sndnet_entry_t* sne, int right);
void leafset_extract(sndnet_entry_t* leafset, const sndnet_entry_t* sne, int right);
void leafset_sort(sndnet_entry_t* leafset, int right);
int leafset_is_on_range(const sndnet_router_t* snr, const sndnet_addr_t* addr);

void sndnet_router_init(sndnet_router_t* snr, const sndnet_addr_t* self) {
    assert(snr != 0);
    assert(self != 0);
    
    memset(snr, 0, sizeof(sndnet_router_t));
    snr->self = *self;
}

void sndnet_router_add(sndnet_router_t* snr, const sndnet_addr_t* addr, const sndnet_realaddr_t* net_addr) {
    sndnet_entry_t e;

    assert(addr != 0);

    e.is_set = 1;
    e.sn_addr = *addr;

    if(net_addr)
        e.net_addr = *net_addr;
    else
        memset(&(e.net_addr), 0, sizeof(sndnet_realaddr_t));

    sndnet_router_set(snr, &e);

    leafset_add(snr, &e);
}

void sndnet_router_remove(sndnet_router_t* snr, const sndnet_addr_t* addr) {
    sndnet_entry_t e;

    assert(addr != 0);

    e.is_set = 0;
    e.sn_addr = *addr;

    sndnet_router_set(snr, &e);

    leafset_remove(snr, &e);
}

void sndnet_router_nexthop(const sndnet_router_t* snr, const sndnet_addr_t* dst, sndnet_entry_t* nexthop) {
    unsigned int level;
    unsigned char column;
    const sndnet_entry_t* e;
    sndnet_entry_t bests[4];
    
    assert(snr != 0);
    assert(dst != 0);
    assert(nexthop != 0);
    
    nexthop->is_set = 0;

    bests[0].is_set = 1;
    bests[0].sn_addr = snr->self;
    
    //Leafset routing
    
    if(leafset_is_on_range(snr, dst)) {
        sndnet_entry_closest(dst, snr->left_leafset, SNDNET_ROUTER_LEAFSET_SIZE, 0, 0, &bests[1]);
        sndnet_entry_closest(dst, snr->right_leafset, SNDNET_ROUTER_LEAFSET_SIZE, 0, 0, &bests[2]);
        sndnet_entry_closest(dst, bests, 3, 0, 0, nexthop);

        if(sndnet_address_cmp(&(nexthop->sn_addr), &(snr->self)) == 0) {
            nexthop->is_set = 0;
        }

        return;
    }
    
    //Table routing
    
    sndnet_address_index(&(snr->self), dst, &level, &column);
    
    if(level < SNDNET_ROUTER_LEVELS && column < SNDNET_ROUTER_COLUMNS)
        e = &(snr->table[level][column]);
    else {
        nexthop->is_set = 0;
        nexthop->sn_addr = snr->self;
        return;
    }
    
    if(e->is_set) {
        *nexthop = *e;
        return;
    }
    
    //Best answer
    
    sndnet_entry_closest(dst, snr->table[level], SNDNET_ROUTER_COLUMNS, 0, 0, &(bests[1]));
    sndnet_entry_closest(dst, snr->left_leafset, SNDNET_ROUTER_LEAFSET_SIZE, &(snr->self), level, &(bests[2]));
    sndnet_entry_closest(dst, snr->right_leafset, SNDNET_ROUTER_LEAFSET_SIZE, &(snr->self), level, &(bests[3]));
    sndnet_entry_closest(dst, bests, 4, 0, 0, nexthop);
    
    if(sndnet_address_cmp(&(nexthop->sn_addr), &(snr->self)) == 0) {
        nexthop->is_set = 0;
    }
}

size_t leafset_get_size(const sndnet_entry_t* leafset) {
    size_t count;

    assert(leafset != 0);

    for(count = 0; count < SNDNET_ROUTER_LEAFSET_SIZE; ++count) {
        if(leafset[count].is_set == 0)
            break;
    }

    return count;
}

int leafset_is_on_range(const sndnet_router_t* snr, const sndnet_addr_t* addr) {
    size_t left_count;
    size_t right_count;
    const sndnet_addr_t *left_bound;
    const sndnet_addr_t *right_bound;
    
    assert(snr != NULL);
    assert(addr != NULL);
    
    left_count = leafset_get_size(snr->left_leafset);
    right_count = leafset_get_size(snr->right_leafset);

    if(left_count)
        left_bound = &snr->left_leafset[left_count].sn_addr;
    else
        left_bound = &snr->self;

    if(right_count)
        right_bound = &snr->right_leafset[right_count].sn_addr;
    else
        right_bound = &snr->self;

    return sndnet_address_cmp(left_bound, addr) <= 0 && sndnet_address_cmp(addr, right_bound) <= 0;
}

void sndnet_router_set(sndnet_router_t* snr, const sndnet_entry_t* sne) {
    unsigned int level;
    unsigned char column;
    sndnet_entry_t* insert;
    const sndnet_addr_t* addr;
    
    assert(snr != 0);
    assert(sne != 0);
    
    addr = &(sne->sn_addr);
    
    sndnet_address_index(&(snr->self), addr, &level, &column);

    insert = &(snr->table[level][column]);

    assert(sndnet_address_cmp(&(snr->self), &(insert->sn_addr)) != 0); //Should be impossible

    *insert = *sne;
}

void leafset_add(sndnet_router_t* snr, const sndnet_entry_t* sne) {
    assert(snr != 0);
    assert(sne != 0);

    if(sndnet_address_cmp(&(sne->sn_addr), &(snr->self)) < 0) {
        leafset_insert(snr->left_leafset, sne, 0);
    } else {
        leafset_insert(snr->right_leafset, sne, 1);
    }
}

void leafset_remove(sndnet_router_t* snr, const sndnet_entry_t* sne) {
    assert(snr != 0);
    assert(sne != 0);

    if(sndnet_address_cmp(&(sne->sn_addr), &(snr->self)) < 0) {
        leafset_extract(snr->left_leafset, sne, 0);
    } else {
        leafset_extract(snr->right_leafset, sne, 1);
    }
}

void leafset_insert(sndnet_entry_t* leafset, const sndnet_entry_t* sne, int right) {
    size_t ls;

    assert(leafset != 0);
    assert(sne != 0);

    ls = leafset_get_size(leafset);

    if(ls < SNDNET_ROUTER_LEAFSET_SIZE) {
        leafset[ls] = *sne;
    } else {
        if((sndnet_address_cmp(&sne->sn_addr, &leafset[SNDNET_ROUTER_LEAFSET_SIZE-1].sn_addr) > 0) ^ right)
            return;

        leafset[SNDNET_ROUTER_LEAFSET_SIZE-1] = *sne;
    }

    leafset_sort(leafset, right);
}

void leafset_extract(sndnet_entry_t* leafset, const sndnet_entry_t* sne, int right) {
    size_t ls;
    int i;

    assert(leafset != 0);
    assert(sne != 0);

    ls = leafset_get_size(leafset);

    for(i = 0; i < ls; ++i) {
        if(sndnet_entry_cmp(sne, &leafset[i]) == 0) {
            leafset[i] = leafset[ls - 1];
            leafset[ls - 1].is_set = 0;

            leafset_sort(leafset, right);

            return;
        }
    }
}

void leafset_sort(sndnet_entry_t* leafset, int right) {
    typedef int (*cmp_t)(const void*, const void*);

    qsort(leafset, leafset_get_size(leafset), sizeof(sndnet_entry_t),
        (cmp_t)(right ? sndnet_entry_cmp : sndnet_entry_cmp_neg));
}