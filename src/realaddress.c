#include "realaddress.h"

#include <assert.h>
#include <netdb.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>

int sndnet_realaddress_from_hostname(sndnet_realaddr_t* snra, const char* hostname, uint16_t port) {
    struct addrinfo hints;
    struct addrinfo *res = 0;
    char port_str[6];
    int ret;

    assert(snra != 0);
    assert(hostname != 0);

    snprintf(port_str, 6, "%hu", port);

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = 0;
    hints.ai_flags = 
        AI_NUMERICHOST |
        AI_NUMERICSERV;

    ret = getaddrinfo(hostname, port_str, &hints, &res);

    if(ret != 0 || res == 0) {
        fprintf(stderr, "%s\n", gai_strerror(ret));
        return -1;
    }

    memcpy(snra, res[0].ai_addr, sizeof(struct sockaddr));

    freeaddrinfo(res);

    return 0;
}

int sndnet_realaddress_from_str(sndnet_realaddr_t* snra, const char* str) {
    char copy_str[23];
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

    return sndnet_realaddress_from_hostname(snra, hostname, port);
}

int sndnet_realaddress_get_hostname(const sndnet_realaddr_t* snra, char* out_hostname, size_t out_hostname_len) {
    assert(snra != 0);
    assert(out_hostname != 0);

    if(getnameinfo(snra, sizeof(struct sockaddr), out_hostname, out_hostname_len, 0, 0, NI_NUMERICHOST | NI_NUMERICSERV))
        return -1;

    return 0;
}

int sndnet_realaddress_get_port(const sndnet_realaddr_t* snra, uint16_t* out_port) {
    char port_str[6];

    assert(snra != 0);
    assert(out_port != 0);

    if(getnameinfo(snra, sizeof(struct sockaddr), 0, 0, port_str, 6, NI_NUMERICHOST | NI_NUMERICSERV))
        return -1;

    if(sscanf(port_str, "%hu", out_port) < 1)
        return -1;

    return 0;
}

int sndnet_realaddress_tostr(const sndnet_realaddr_t* snra, char* out_str, size_t out_str_len) {
    char host_str[16];
    char port_str[6];
    int written;

    assert(snra != 0);
    assert(out_str != 0);

    if(getnameinfo(snra, sizeof(struct sockaddr), host_str, 16, port_str, 6, NI_NUMERICHOST | NI_NUMERICSERV))
        return -1;

    written = snprintf(out_str, out_str_len, "%s:%s", host_str, port_str);

    if(written < (strlen(host_str) + strlen(port_str) + 1))
        return -1;

    return 0;
}