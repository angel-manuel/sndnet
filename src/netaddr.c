#include "netaddr.h"

#include <string.h>

int sn_netaddr_bind(const sn_netaddr_t* na, int fd) {
    assert(na != 0);
    assert(fd >= 0);

    switch (na->sa_family) {
        case AF_INET:
            return sn_realaddr_bind((sn_realaddr_t*)na, fd);
        case AF_LOCAL:
            return sn_localaddr_bind((sn_localaddr_t*)na, fd);
        default:
            return -1;
    }

    return -1;
}

socklen_t sn_netaddr_get_size(const sn_netaddr_t* na) {
    assert(na != 0);

    switch (na->sa_family) {
        case AF_INET:
            return sizeof(sn_realaddr_t);
        case AF_LOCAL:
            return sizeof(sn_localaddr_t);
        default:
            return sizeof(struct sockaddr);
    }

    return 0;
}

int sn_netaddr_cmp(const sn_netaddr_t* a, const sn_netaddr_t* b) {
    assert(a != 0);
    assert(b != 0);

    if(a->sa_family != b->sa_family)
        return a->sa_family - b->sa_family;

    switch (a->sa_family) {
        case AF_INET:
            return sn_realaddr_cmp((sn_realaddr_t*)a, (sn_realaddr_t*)b);
        case AF_LOCAL:
            return sn_localaddr_cmp((sn_localaddr_t*)a, (sn_localaddr_t*)b);
        default:
            return 0;
    }
}

int sn_netaddr_tostr(const sn_netaddr_t* na, char* out_str) {
    assert(na != 0);
    assert(out_str != 0);

    switch (na->sa_family) {
        case AF_INET:
            strcpy(out_str, "INET:");
            return sn_realaddr_tostr((sn_realaddr_t*)na, out_str + 5);
            break;
        case AF_LOCAL:
            strcpy(out_str, "UNIX:");
            sn_localaddr_tostr((sn_localaddr_t*)na, out_str + 5);
            return 0;
            break;
        default:
            strcpy(out_str, "NONE");
            return 0;
    }

    return -1;
}

int sn_netaddr_ser(const sn_netaddr_t* na, sn_netaddr_ser_t* ser) {
    assert(na != 0);
    assert(ser != 0);

    ser->type = (int32_t)na->sa_family;
    switch (na->sa_family) {
        case AF_INET:
            return sn_realaddr_ser((sn_realaddr_t*)na, &ser->realaddr);
        case AF_LOCAL:
            return sn_localaddr_ser((sn_localaddr_t*)na, &ser->localaddr);
        default:
            return -1;
    }
}

int sn_netaddr_deser(sn_netaddr_t* na, const sn_netaddr_ser_t* ser) {
    assert(na != 0);
    assert(ser != 0);

    na->sa_family = (int)ser->type;
    switch (na->sa_family) {
        case AF_INET:
            return sn_realaddr_deser((sn_realaddr_t*)na, &ser->realaddr);
        case AF_LOCAL:
            return sn_localaddr_deser((sn_localaddr_t*)na, &ser->localaddr);
        default:
            return -1;
    }
}
