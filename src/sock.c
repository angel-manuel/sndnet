#include "sock.h"

#include <assert.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

int sn_sock_init(sn_sock_t* ss, int type) {
    int fd;

    assert(ss != 0);

    fd = socket(type, SOCK_DGRAM, 0);

    if(fd == -1)
        return -1;

    ss->fd = fd;
    ss->type = type;
    ss->binded = 0;

    return 0;
}

int sn_sock_init_binded(sn_sock_t* ss, sn_netaddr_t* na) {
    assert(ss != 0);
    assert(na != 0);

    if(sn_sock_init(ss, na->sa_family) == -1)
        return -1;

    return sn_sock_bind(ss, na);
}

int sn_sock_destroy(sn_sock_t* ss) {
    assert(ss != 0);

    return close(ss->fd);
}

int sn_sock_bind(sn_sock_t* ss, sn_netaddr_t* na) {
    assert(ss != 0);
    assert(na != 0);

    if(sn_netaddr_bind(na, ss->fd) == -1)
        return -1;

    ss->bind = *na;
    ss->binded = 1;

    return 0;
}

ssize_t sn_sock_send(sn_sock_t* ss, const void* buf, size_t len, int flags, const sn_netaddr_t* dst_addr) {
    assert(ss != 0);
    assert(buf != 0 || len == 0);

    return sendto(ss->fd, buf, len, flags, dst_addr, sn_netaddr_get_size(dst_addr));
}

ssize_t sn_sock_recv(sn_sock_t* ss, void* buf, size_t len, int flags, sn_netaddr_t* src_addr) {
    ssize_t ret;

    assert(ss != 0);
    assert(buf != 0 || len == 0);

    if(src_addr) {
        socklen_t addrlen = sizeof(struct sockaddr);

        ret = recvfrom(ss->fd, buf, len, flags, src_addr, &addrlen);
    } else
        ret = recvfrom(ss->fd, buf, len, flags, 0, 0);

    return ret;
}
