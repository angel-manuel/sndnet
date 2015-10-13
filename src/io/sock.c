#include "io/sock.h"

#include <assert.h>
#include <errno.h>
#include <stddef.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

sn_io_sock_t sn_io_sock_named(const sn_io_naddr_t* name) {
    int fd;

    assert(name != NULL);

    fd = socket(name->sa_family, SOCK_DGRAM, 0);

    if(fd == -1)
        return SN_IO_SOCK_INVALID;

    if(bind(fd, name, sizeof(*name)) == -1) {
        sn_io_sock_close(fd);
        return SN_IO_SOCK_INVALID;
    }

    return fd;
}

void sn_io_sock_close(sn_io_sock_t socket) {
    int ret;

    assert(socket != SN_IO_SOCK_INVALID);

    do {
        ret = close(socket);
    } while(ret == -1 && errno == EINTR);
}

int sn_io_sock_get_name(sn_io_sock_t socket, sn_io_naddr_t* out_name) {
    socklen_t addrlen;

    assert(socket != SN_IO_SOCK_INVALID);
    assert(out_name != NULL);

    addrlen = sizeof(*out_name);

    return getsockname(socket, out_name, &addrlen);
}

ssize_t sn_io_sock_send(sn_io_sock_t socket, const void* buf, size_t len, const sn_io_naddr_t* dst) {
    assert(socket != SN_IO_SOCK_INVALID);
    assert(buf != NULL);
    assert(dst != NULL);

    return sendto(socket, buf, len, 0, dst, sizeof(*dst));
}

ssize_t sn_io_sock_recv(sn_io_sock_t socket, void* buf, size_t len, sn_io_naddr_t* src) {
    socklen_t addrlen;

    assert(socket != SN_IO_SOCK_INVALID);
    assert(buf != NULL || len == 0);

    addrlen = src != NULL ? sizeof(*src) : 0;

    return recvfrom(socket, buf, len, 0, src, &addrlen);
}

ssize_t sn_io_sock_peek(sn_io_sock_t socket, void* buf, size_t len, sn_io_naddr_t* src) {
    socklen_t addrlen;

    assert(socket != SN_IO_SOCK_INVALID);
    assert(buf != NULL);

    addrlen = src != NULL ? sizeof(*src) : 0;

    return recvfrom(socket, buf, len, MSG_PEEK, src, &addrlen);
}
