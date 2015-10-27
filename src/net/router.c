#include "net/router.h"

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

void sn_net_router_set(sn_net_router_t* snr, const sn_net_entry_t* sne);
void leafset_add(sn_net_router_t* snr, const sn_net_entry_t* sne);
void leafset_remove(sn_net_router_t* snr, const sn_net_entry_t* sne);
void leafset_insert(sn_net_entry_t* leafset, const sn_net_entry_t* sne, int right);
void leafset_extract(sn_net_entry_t* leafset, const sn_net_entry_t* sne, int right);
void leafset_sort(sn_net_entry_t* leafset, int right);
int leafset_is_on_range(const sn_net_router_t* snr, const sn_net_addr_t* addr);

void sn_net_router_init(sn_net_router_t* snr, const sn_net_addr_t* self_addr, const sn_io_naddr_t* self_net_addr) {
    assert(snr != NULL);
    assert(self_addr != NULL);
    assert(self_net_addr != NULL);

    memset(snr, 0, sizeof(sn_net_router_t));
    snr->self.is_set = 1;
    snr->self.sn_net_addr = *self_addr;
    snr->self.net_addr = *self_net_addr;
}

void sn_net_router_add(sn_net_router_t* snr, const sn_net_addr_t* addr, const sn_io_naddr_t* net_addr) {
    sn_net_entry_t e;

    assert(addr != NULL);

    e.is_set = 1;
    e.sn_net_addr = *addr;

    if(net_addr != NULL)
        e.net_addr = *net_addr;
    else
        memset(&(e.net_addr), 0, sizeof(sn_io_naddr_t));

    sn_net_router_set(snr, &e);

    leafset_add(snr, &e);
}

void sn_net_router_remove(sn_net_router_t* snr, const sn_net_addr_t* addr) {
    sn_net_entry_t e;

    assert(addr != NULL);

    e.is_set = 0;
    e.sn_net_addr = *addr;

    sn_net_router_set(snr, &e);

    leafset_remove(snr, &e);
}

void sn_net_router_nexthop(const sn_net_router_t* snr, const sn_net_addr_t* dst, sn_net_entry_t* nexthop) {
    unsigned int level;
    unsigned char column;
    const sn_net_entry_t* e;
    sn_net_entry_t bests[4];

    assert(snr != NULL);
    assert(dst != NULL);
    assert(nexthop != NULL);

    nexthop->is_set = 0;

    bests[0] = snr->self;

    //Leafset routing

    if(leafset_is_on_range(snr, dst)) {
        sn_net_entry_closest(dst, snr->left_leafset, SN_NET_ROUTER_LEAFSET_SIZE, 0, 0, &bests[1]);
        sn_net_entry_closest(dst, snr->right_leafset, SN_NET_ROUTER_LEAFSET_SIZE, 0, 0, &bests[2]);
        sn_net_entry_closest(dst, bests, 3, 0, 0, nexthop);

        if(sn_net_entry_cmp(nexthop, &snr->self) == 0) {
            nexthop->is_set = 0;
        }

        return;
    }

    //Table routing

    sn_net_addr_index(&snr->self.sn_net_addr, dst, &level, &column);

    if(level < SN_NET_ROUTER_LEVELS && column < SN_NET_ROUTER_COLUMNS)
        e = &(snr->table[level][column]);
    else {
        *nexthop = snr->self;
        nexthop->is_set = 0;
        return;
    }

    if(e->is_set) {
        *nexthop = *e;
        return;
    }

    //Best answer

    sn_net_entry_closest(dst, snr->table[level], SN_NET_ROUTER_COLUMNS, 0, 0, &(bests[1]));
    sn_net_entry_closest(dst, snr->left_leafset, SN_NET_ROUTER_LEAFSET_SIZE, &snr->self.sn_net_addr, level, &(bests[2]));
    sn_net_entry_closest(dst, snr->right_leafset, SN_NET_ROUTER_LEAFSET_SIZE, &snr->self.sn_net_addr, level, &(bests[3]));
    sn_net_entry_closest(dst, bests, 4, 0, 0, nexthop);

    if(sn_net_entry_cmp(nexthop, &snr->self) == 0) {
        nexthop->is_set = 0;
    }
}

int sn_net_router_to_str(const sn_net_router_t* snr, char* out_str, size_t out_str_len) {
    size_t used_len = 0;

    assert(snr != NULL);
    assert(out_str != NULL);

    {
        char self_str[SN_NET_ENTRY_PRINTABLE_LEN];

        sn_net_entry_to_str(&snr->self, self_str, SN_NET_ADDR_HEX_LEN);

        used_len += snprintf(out_str + used_len, out_str_len - used_len,
        "Router is at %s\n", self_str);
    }

    {/*Left leafset*/
        size_t left_leafset_size = sn_net_entry_array_len(snr->left_leafset, SN_NET_ROUTER_LEAFSET_SIZE);
        unsigned int i;

        used_len += snprintf(out_str + used_len, out_str_len - used_len,
        "Left leafset[%lu]\n", (uint64_t)left_leafset_size);

        if(used_len >= out_str_len)
            return -1;

        for(i = 0; i < left_leafset_size; ++i) {
            char entry[SN_NET_ENTRY_PRINTABLE_LEN];

            if(sn_net_entry_to_str(&snr->left_leafset[i], entry, 16))
                return -1;

            used_len += snprintf(out_str + used_len, out_str_len - used_len,
            "\t%s\n", entry);
        }
    }

    if(used_len >= out_str_len)
        return -1;

    {/*Right leafset*/
        size_t right_leafset_size = sn_net_entry_array_len(snr->right_leafset, SN_NET_ROUTER_LEAFSET_SIZE);
        unsigned int i;

        used_len += snprintf(out_str + used_len, out_str_len - used_len,
        "Right leafset[%lu]\n", (uint64_t)right_leafset_size);

        if(used_len >= out_str_len)
            return -1;

        for(i = 0; i < right_leafset_size; ++i) {
            char entry[SN_NET_ENTRY_PRINTABLE_LEN];

            if(sn_net_entry_to_str(&snr->right_leafset[i], entry, 16))
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

        for(level = 0; level < SN_NET_ROUTER_LEVELS; ++level) {
            unsigned int column;

            for(column = 0; column < SN_NET_ROUTER_COLUMNS; ++column) {
                if(snr->table[level][column].is_set) {
                    char entry[SN_NET_ENTRY_PRINTABLE_LEN];

                    if(sn_net_entry_to_str(&snr->table[level][column], entry, 16))
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

const sn_net_entry_t* sn_net_router_table_get(const sn_net_router_t* snr, unsigned int level, unsigned int column) {
    assert(snr != NULL);
    assert(level <= SN_NET_ROUTER_LEVELS);
    assert(column <= SN_NET_ROUTER_COLUMNS);

    return &snr->table[level][column];
}

const sn_net_entry_t* sn_net_router_leafset_get(const sn_net_router_t* snr, int position) {
    assert(snr != NULL);
    assert(position <= SN_NET_ROUTER_LEAFSET_SIZE);
    assert(-SN_NET_ROUTER_LEAFSET_SIZE <= position);

    if(position > 0) {
        return &snr->right_leafset[position - 1];
    } else if(position < 0) {
        return &snr->left_leafset[-position - 1];
    } else {
        return &snr->self;
    }
}

void sn_net_router_table_set(sn_net_router_t* snr, unsigned int level, unsigned int column, const sn_net_entry_t* e) {
    assert(snr != NULL);
    assert(level < SN_NET_ROUTER_LEVELS);
    assert(column < SN_NET_ROUTER_COLUMNS);
    assert(e != NULL);

    snr->table[level][column] = *e;
}

void sn_net_router_leafset_set(sn_net_router_t* snr, int position, const sn_net_entry_t* e) {
    assert(snr != NULL);
    assert(position <= SN_NET_ROUTER_LEAFSET_SIZE);
    assert(-SN_NET_ROUTER_LEAFSET_SIZE <= position);
    assert(e != NULL);

    if(position > 0) {
        snr->right_leafset[position - 1] = *e;
    } else if(position < 0) {
        snr->left_leafset[-position - 1] = *e;
    } else {
        snr->self = *e;
    }
}

size_t sn_net_router_query_table(const sn_net_router_t* snr, uint16_t l_min, uint16_t l_max, sn_net_router_query_ser_t** out_query) {
    size_t max_size;
    size_t final_size;
    sn_net_router_query_ser_t* ret;

    assert(snr != NULL);
    assert(out_query != NULL);

    if(l_min >= SN_NET_ROUTER_LEVELS ||
       l_max >= SN_NET_ROUTER_LEVELS ||
       l_max < l_min)
       return 0;

    max_size = (l_max - l_min)*SN_NET_ROUTER_COLUMNS + 1;

    ret = (sn_net_router_query_ser_t*)malloc(sizeof(sn_net_router_query_ser_t) + max_size*sizeof(sn_net_router_entry_ser_t));

    if(ret == 0)
        return 0;

    ret->entries_len = 0;

    for(uint16_t l = l_min; l <= l_max; ++l)
        for(uint16_t c = 0; c < SN_NET_ROUTER_COLUMNS; ++c) {
            const sn_net_entry_t* e = sn_net_router_table_get(snr, l, c);

            if(e && e->is_set) {
                sn_net_router_entry_ser_t* e_ser = &ret->entries[ret->entries_len];

                if(sn_net_entry_ser(e, &e_ser->entry) == 0) {
                    e_ser->is_table = 1;
                    e_ser->level = l;
                    e_ser->column = c;

                    ++ret->entries_len;
                }
            }
        }


    final_size = sizeof(sn_net_router_query_ser_t) + (ret->entries_len)*sizeof(sn_net_router_entry_ser_t);
    *out_query = (sn_net_router_query_ser_t*)realloc(ret, final_size);

    if(*out_query == 0) {
        free(ret);
        return 0;
    }

    return final_size;
}

size_t sn_net_router_query_leafset(const sn_net_router_t* snr, int32_t p_min, int32_t p_max, sn_net_router_query_ser_t** out_query) {
    size_t max_size;
    size_t final_size;
    sn_net_router_query_ser_t* ret;

    assert(snr != NULL);
    assert(out_query != NULL);

    if(p_min < -SN_NET_ROUTER_LEAFSET_SIZE ||
       p_max > SN_NET_ROUTER_LEAFSET_SIZE ||
       p_max < p_min)
       return 0;

    max_size = p_max - p_min + 1;

    ret = (sn_net_router_query_ser_t*)malloc(sizeof(sn_net_router_query_ser_t) + max_size*sizeof(sn_net_router_entry_ser_t));

    if(ret == NULL)
        return 0;

    ret->entries_len = 0;

    for(int32_t p = p_min; p <= p_max; ++p) {
        const sn_net_entry_t* e = sn_net_router_leafset_get(snr, p);

        if(e && e->is_set) {
            sn_net_router_entry_ser_t* e_ser = &ret->entries[ret->entries_len];

            if(sn_net_entry_ser(e, &e_ser->entry) == 0) {
                e_ser->is_table = 0;
                e_ser->position = p;

                ++ret->entries_len;
            }
        }
    }

    final_size = sizeof(sn_net_router_query_ser_t) + (ret->entries_len)*sizeof(sn_net_router_entry_ser_t);
    *out_query = (sn_net_router_query_ser_t*)realloc(ret, final_size);

    if(*out_query == NULL) {
        free(ret);
        return 0;
    }

    return final_size;
}

/* Private functions */

int leafset_is_on_range(const sn_net_router_t* snr, const sn_net_addr_t* addr) {
    size_t left_count;
    size_t right_count;
    const sn_net_addr_t *left_bound;
    const sn_net_addr_t *right_bound;

    assert(snr != NULL);
    assert(addr != NULL);

    left_count = sn_net_entry_array_len(snr->left_leafset, SN_NET_ROUTER_LEAFSET_SIZE);
    right_count = sn_net_entry_array_len(snr->right_leafset, SN_NET_ROUTER_LEAFSET_SIZE);

    if(left_count)
        left_bound = &snr->left_leafset[left_count].sn_net_addr;
    else
        left_bound = &snr->self.sn_net_addr;

    if(right_count)
        right_bound = &snr->right_leafset[right_count].sn_net_addr;
    else
        right_bound = &snr->self.sn_net_addr;

    return sn_net_addr_cmp(left_bound, addr) <= 0 && sn_net_addr_cmp(addr, right_bound) <= 0;
}

void sn_net_router_set(sn_net_router_t* snr, const sn_net_entry_t* sne) {
    unsigned int level;
    unsigned char column;
    sn_net_entry_t* insert;
    const sn_net_addr_t* addr;

    assert(snr != NULL);
    assert(sne != NULL);

    addr = &(sne->sn_net_addr);

    sn_net_addr_index(&snr->self.sn_net_addr, addr, &level, &column);

    insert = &(snr->table[level][column]);

    assert(sn_net_addr_cmp(&snr->self.sn_net_addr, &insert->sn_net_addr) != 0); //Should be impossible

    *insert = *sne;
}

void leafset_add(sn_net_router_t* snr, const sn_net_entry_t* sne) {
    assert(snr != NULL);
    assert(sne != NULL);

    if(sn_net_entry_cmp(sne, &snr->self) < 0) {
        leafset_insert(snr->left_leafset, sne, 0);
    } else {
        leafset_insert(snr->right_leafset, sne, 1);
    }
}

void leafset_remove(sn_net_router_t* snr, const sn_net_entry_t* sne) {
    assert(snr != NULL);
    assert(sne != NULL);

    if(sn_net_entry_cmp(sne, &snr->self) < 0) {
        leafset_extract(snr->left_leafset, sne, 0);
    } else {
        leafset_extract(snr->right_leafset, sne, 1);
    }
}

void leafset_insert(sn_net_entry_t* leafset, const sn_net_entry_t* sne, int right) {
    size_t ls;

    assert(leafset != NULL);
    assert(sne != NULL);

    ls = sn_net_entry_array_len(leafset, SN_NET_ROUTER_LEAFSET_SIZE);

    if(ls < SN_NET_ROUTER_LEAFSET_SIZE) {
        leafset[ls] = *sne;
    } else {
        if((sn_net_addr_cmp(&sne->sn_net_addr, &leafset[SN_NET_ROUTER_LEAFSET_SIZE-1].sn_net_addr) < 0) ^ right)
            return;

        leafset[SN_NET_ROUTER_LEAFSET_SIZE-1] = *sne;
    }

    leafset_sort(leafset, right);
}

void leafset_extract(sn_net_entry_t* leafset, const sn_net_entry_t* sne, int right) {
    size_t ls;
    unsigned int i;

    assert(leafset != NULL);
    assert(sne != NULL);

    ls = sn_net_entry_array_len(leafset, SN_NET_ROUTER_LEAFSET_SIZE);

    for(i = 0; i < ls; ++i) {
        if(sn_net_entry_cmp(sne, &leafset[i]) == 0) {
            leafset[i] = leafset[ls - 1];
            leafset[ls - 1].is_set = 0;

            leafset_sort(leafset, right);

            return;
        }
    }
}

void leafset_sort(sn_net_entry_t* leafset, int right) {
    typedef int (*cmp_t)(const void*, const void*);

    qsort(leafset, sn_net_entry_array_len(leafset, SN_NET_ROUTER_LEAFSET_SIZE), sizeof(sn_net_entry_t),
        (cmp_t)(right ? sn_net_entry_cmp : sn_net_entry_cmp_neg));
}
