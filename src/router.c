#include "router.h"

#include <assert.h>
#include <stdio.h>
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

int sndnet_router_tostr(const sndnet_router_t* snr, char* out_str, size_t out_str_len) {
    size_t used_len = 0;
    
    assert(snr != 0);
    assert(out_str != 0);
    
    {
        char self_str[SNDNET_ADDRESS_PRINTABLE_LENGTH];
        
        sndnet_address_tostr(&snr->self, self_str);
        
        used_len += snprintf(out_str + used_len, out_str_len - used_len,
        "Router @ %s\n", self_str);
    }
    
    {/*Left leafset*/
        size_t left_leafset_size = sndnet_entry_array_len(snr->left_leafset);
        unsigned int i;
        
        used_len += snprintf(out_str + used_len, out_str_len - used_len,
        "Left leafset[%lu]\n", (uint64_t)left_leafset_size);
        
        if(used_len >= out_str_len)
            return -1;
        
        for(i = 0; i < left_leafset_size; ++i) {
            char entry[SNDNET_ENTRY_PRINTABLE_LENGTH];
            
            if(sndnet_entry_tostr(&snr->left_leafset[i], entry, 16))
                return -1;
            
            used_len += snprintf(out_str + used_len, out_str_len - used_len,
            "\t%s\n", entry);
        }
    }
    
    if(used_len >= out_str_len)
        return -1;
    
    {/*Right leafset*/
        size_t right_leafset_size = sndnet_entry_array_len(snr->right_leafset);
        unsigned int i;
        
        used_len += snprintf(out_str + used_len, out_str_len - used_len,
        "Right leafset[%lu]\n", (uint64_t)right_leafset_size);
        
        if(used_len >= out_str_len)
            return -1;
        
        for(i = 0; i < right_leafset_size; ++i) {
            char entry[SNDNET_ENTRY_PRINTABLE_LENGTH];
            
            if(sndnet_entry_tostr(&snr->right_leafset[i], entry, 16))
                return -1;
            
            used_len += snprintf(out_str + used_len, out_str_len - used_len,
            "\t%s\n", entry);
        }
    }
    
    if(used_len >= out_str_len)
        return -1;
    
    {/*Routing table*/
        unsigned int level;
        
        used_len += snprintf(out_str + used_len, out_str_len - used_len,
        "Routing table\n");
        
        for(level = 0; level < SNDNET_ROUTER_LEVELS; ++level) {
            unsigned int column;
            
            for(column = 0; column < SNDNET_ROUTER_COLUMNS; ++column) {
                if(snr->table[level][column].is_set) {
                    char entry[SNDNET_ENTRY_PRINTABLE_LENGTH];
                    
                    if(sndnet_entry_tostr(&snr->table[level][column], entry, 16))
                        return -1;
                        
                    used_len += snprintf(out_str + used_len, out_str_len - used_len,
                    "\t[%2u][%2u]:%s\n", level, column, entry);
                }
            }
        }
    }
    
    if(used_len >= out_str_len)
        return -1;
    
    return 0;
}

int leafset_is_on_range(const sndnet_router_t* snr, const sndnet_addr_t* addr) {
    size_t left_count;
    size_t right_count;
    const sndnet_addr_t *left_bound;
    const sndnet_addr_t *right_bound;
    
    assert(snr != NULL);
    assert(addr != NULL);
    
    left_count = sndnet_entry_array_len(snr->left_leafset);
    right_count = sndnet_entry_array_len(snr->right_leafset);

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

    ls = sndnet_entry_array_len(leafset);

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

    ls = sndnet_entry_array_len(leafset);

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

    qsort(leafset, sndnet_entry_array_len(leafset), sizeof(sndnet_entry_t),
        (cmp_t)(right ? sndnet_entry_cmp : sndnet_entry_cmp_neg));
}