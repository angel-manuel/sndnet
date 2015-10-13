#include "io/naddr.h"

#include <assert.h>
#include <stddef.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <string.h>
#include <strings.h>
#include <stdio.h>

int sn_io_naddr_ipv4(sn_io_naddr_t* addr, const char* ipv4, uint16_t port) {
    struct sockaddr_in* inaddr;

    assert(addr != NULL);
    assert(ipv4 != NULL);

    inaddr = (struct sockaddr_in*)addr;

    memset(inaddr, 0, sizeof(sn_io_naddr_t));
    inaddr->sin_family = AF_INET;
    inaddr->sin_port = htons(port);
    if(inet_pton(AF_INET, ipv4, &inaddr->sin_addr) <= 0)
        return -1;

    return 0;
}

int sn_io_naddr_local(sn_io_naddr_t* addr, const char* path) {
    struct sockaddr_un* unaddr;

    assert(addr != NULL);
    assert(path != NULL);

    unaddr = (struct sockaddr_un*)addr;

    unaddr->sun_family = AF_UNIX;
    memset(unaddr->sun_path, 0, SN_IO_NADDR_MAX_PATH_LEN);

    strncpy(unaddr->sun_path+1, path, SN_IO_NADDR_MAX_PATH_LEN);

    return 0;
}

int sn_io_naddr_from_str(sn_io_naddr_t* addr, const char* str) {
    assert(addr != NULL);
    assert(str != NULL);

    if(strncasecmp(str, "INET", 4) == 0) {
        char copy_str[SN_IO_NADDR_PRINTABLE_LEN];
        char *saveptr, *ipv4, *port;
        uint16_t nport;

        strncpy(copy_str, str, SN_IO_NADDR_PRINTABLE_LEN);

        ipv4 = strtok_r(copy_str+5, ":", &saveptr);

        if(ipv4 == NULL)
            return -1;

        port = strtok_r(NULL, ":", &saveptr);

        if(port == NULL)
            return -1;

        if(sscanf(port, "%hu", &nport) < 1)
            return -1;

        return sn_io_naddr_ipv4(addr, ipv4, nport);
    } else if(strncasecmp(str, "UNIX", 4) == 0) {
        char copy_str[SN_IO_NADDR_MAX_PATH_LEN];

        strncpy(copy_str, str, SN_IO_NADDR_MAX_PATH_LEN);

        if(copy_str[5] == '_')
            copy_str[5] = '\0';
        return sn_io_naddr_local(addr, copy_str+5);
    } else {
        return -1;
    }
}

int sn_io_naddr_to_str(const sn_io_naddr_t* addr, char* out_str) {
    assert(addr != NULL);
    assert(out_str != NULL);

    switch (addr->sa_family) {
        case AF_INET:
            {
                char inet_str[INET_ADDRSTRLEN];
                if(inet_ntop(AF_INET, &((struct sockaddr_in*)addr)->sin_addr, inet_str, INET_ADDRSTRLEN) == NULL)
                    return -1;
                snprintf(out_str, SN_IO_NADDR_PRINTABLE_LEN, "INET:%s:%hu", inet_str, ntohs(((struct sockaddr_in*)addr)->sin_port));
            }
            break;
        case AF_UNIX:
            {
                char path_str[SN_IO_NADDR_MAX_PATH_LEN];
                strncpy(path_str, ((struct sockaddr_un*)addr)->sun_path, SN_IO_NADDR_MAX_PATH_LEN);
                path_str[SN_IO_NADDR_MAX_PATH_LEN-1] = '\0';
                snprintf(out_str, SN_IO_NADDR_PRINTABLE_LEN, "UNIX:%s", path_str);
            }
            break;
        default:
            snprintf(out_str, SN_IO_NADDR_PRINTABLE_LEN, "NONE");
    }

    return 0;
}

int sn_io_naddr_ser(const sn_io_naddr_t* addr, sn_io_naddr_ser_t* ser) {
    assert(addr != NULL);
    assert(ser != NULL);

    switch (addr->sa_family) {
        case AF_INET:
            ser->type = 1;
            ser->inet.ipv4 = ((struct sockaddr_in*)addr)->sin_addr.s_addr;
            break;
        case AF_UNIX:
            ser->type = 2;
            memcpy(ser->local_path, ((struct sockaddr_un*)addr)->sun_path, SN_IO_NADDR_MAX_PATH_LEN);
            break;
        default:
            return -1;
    }

    return 0;
}

int sn_io_naddr_deser(sn_io_naddr_t* addr, const sn_io_naddr_ser_t* ser) {
    assert(addr != NULL);
    assert(ser != NULL);

    switch (ser->type) {
        case 1:
            ((struct sockaddr_in*)addr)->sin_family = AF_INET;
            ((struct sockaddr_in*)addr)->sin_port = ser->inet.port;
            ((struct sockaddr_in*)addr)->sin_addr.s_addr = ser->inet.ipv4;
            break;
        case 2:
            ((struct sockaddr_un*)addr)->sun_family = AF_UNIX;
            memcpy(((struct sockaddr_un*)addr)->sun_path, ser->local_path, SN_IO_NADDR_MAX_PATH_LEN);
            break;
        default:
            return -1;
    }

    return 0;
}
