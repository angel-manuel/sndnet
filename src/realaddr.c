#include "realaddr.h"

#include <assert.h>
#include <netdb.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>

int realaddr_from_hostname_with_flags(sn_realaddr_t* snra, const char* hostname, uint16_t port, int flags);

int sn_realaddr_from_hostname(sn_realaddr_t* snra, const char* hostname, uint16_t port) {
    assert(snra != 0);
    assert(hostname != 0);

    return realaddr_from_hostname_with_flags(snra, hostname, port, AI_NUMERICHOST | AI_NUMERICSERV);
}

int sn_realaddr_local_at_port(sn_realaddr_t* snra, uint16_t port) {
    assert(snra != 0);

    return realaddr_from_hostname_with_flags(snra, 0, port, AI_PASSIVE);
}

int sn_realaddr_from_str(sn_realaddr_t* snra, const char* str) {
    char copy_str[SN_REALADDR_PRINTABLE_LEN];
    char* hostname;
    char* port_str;
    uint16_t port;

    assert(str != 0);

    strncpy(copy_str, str, 23);

    hostname = strtok(copy_str, ":");

    if(!hostname)
        return -1;

    port_str = strtok(0, "");

    if(!port_str)
        return -1;

    if(sscanf(port_str, "%hu", &port) < 1)
        return -1;

    return sn_realaddr_from_hostname(snra, hostname, port);
}

int sn_realaddr_bind(const sn_realaddr_t* snra, int socket_fd) {
    assert(snra != 0);
    assert(socket_fd > 0);

    return bind(socket_fd, snra, sizeof(*snra));
}

int sn_realaddr_get_hostname(const sn_realaddr_t* snra, char* out_hostname) {
    assert(snra != 0);
    assert(out_hostname != 0);

    if(getnameinfo(snra, sizeof(struct sockaddr), out_hostname, SN_REALADDR_HOSTNAME_PRINTABLE_LEN, 0, 0, NI_NUMERICHOST | NI_NUMERICSERV))
        return -1;

    return 0;
}

int sn_realaddr_get_port(const sn_realaddr_t* snra, uint16_t* out_port) {
    char port_str[SN_REALADDR_PORT_PRINTABLE_LEN];

    assert(snra != 0);
    assert(out_port != 0);

    if(getnameinfo(snra, sizeof(struct sockaddr), 0, 0, port_str, 6, NI_NUMERICHOST | NI_NUMERICSERV))
        return -1;

    if(sscanf(port_str, "%hu", out_port) < 1)
        return -1;

    return 0;
}

int sn_realaddr_tostr(const sn_realaddr_t* snra, char* out_str) {
    char host_str[SN_REALADDR_HOSTNAME_PRINTABLE_LEN];
    char port_str[SN_REALADDR_PORT_PRINTABLE_LEN];
    int written;

    assert(snra != 0);
    assert(out_str != 0);

    if(getnameinfo(snra, sizeof(struct sockaddr), host_str, 16, port_str, 6, NI_NUMERICHOST | NI_NUMERICSERV))
        return -1;

    written = snprintf(out_str, SN_REALADDR_PRINTABLE_LEN, "%s:%s", host_str, port_str);

    if(written < (strlen(host_str) + strlen(port_str) + 1))
        return -1;

    return 0;
}

int sn_realaddr_cmp(const sn_realaddr_t* a, const sn_realaddr_t* b) {
    char a_str[SN_REALADDR_PRINTABLE_LEN];
    char b_str[SN_REALADDR_PRINTABLE_LEN];
    uint8_t aa, ab, ac, ad, ba, bb, bc, bd;
    uint16_t ap, bp;

    assert(a != 0);
    assert(b != 0);

    sn_realaddr_tostr(a, a_str);
    sn_realaddr_tostr(b, b_str);

    sscanf(a_str, "%hhu.%hhu.%hhu.%hhu:%hu", &aa, &ab, &ac, &ad, &ap);
    sscanf(b_str, "%hhu.%hhu.%hhu.%hhu:%hu", &ba, &bb, &bc, &bd, &bp);

    if(aa == ba) {
        if(ab == bb) {
            if(ac == bc) {
                if(ad == bd)
                    return (int)ap - bp;
                else
                    return (int)ad - bd;
            } else
                return (int)ac - bc;
        } else
            return (int)ab - bb;
    } else
        return (int)aa - ba;
}

int sn_realaddr_ser(const sn_realaddr_t* snra, sn_realaddr_ser_t* ser) {
    char hostname[SN_REALADDR_HOSTNAME_PRINTABLE_LEN];
    uint32_t a, b, c, d;

    assert(snra != 0);
    assert(ser != 0);

    if(sn_realaddr_get_hostname(snra, hostname) < 0)
        return -1;

    if(sscanf(hostname, "%u.%u.%u.%u", &a, &b, &c, &d) < 4)
        return -1;

    ser->ipv4 = a<<24 | b<<16 | c<<8 | d;

    if(sn_realaddr_get_port(snra, &ser->port) < 0)
        return -1;

    return 0;
}

int sn_realaddr_deser(sn_realaddr_t* snra, const sn_realaddr_ser_t* ser) {
    char hostname[SN_REALADDR_HOSTNAME_PRINTABLE_LEN];
    uint32_t a, b, c, d;

    assert(snra != 0);
    assert(ser != 0);

    a = (ser->ipv4 & 0xff000000) >> 24;
    b = (ser->ipv4 & 0x00ff0000) >> 16;
    c = (ser->ipv4 & 0x0000ff00) >> 8;
    d = (ser->ipv4 & 0x000000ff);

    if(snprintf(hostname, SN_REALADDR_HOSTNAME_PRINTABLE_LEN, "%u.%u.%u.%u", a, b, c, d) < 0)
        return -1;

    return sn_realaddr_from_hostname(snra, hostname, ser->port);
}

/* Private functions */

int realaddr_from_hostname_with_flags(sn_realaddr_t* snra, const char* hostname, uint16_t port, int flags) {
    struct addrinfo hints;
    struct addrinfo *res = 0;
    char port_str[6];
    int ret;

    assert(snra != 0);

    snprintf(port_str, 6, "%hu", port);

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = 0;
    hints.ai_flags = flags;

    ret = getaddrinfo(hostname, port_str, &hints, &res);

    if(ret != 0 || res == 0) {
        fprintf(stderr, "%s\n", gai_strerror(ret));
        return -1;
    }

    *snra = *res[0].ai_addr;

    freeaddrinfo(res);

    return 0;
}
