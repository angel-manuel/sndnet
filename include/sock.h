/**
 * @file
 * Provides functions for working with sockets
 * */

#ifndef SN_SOCK_H_
#define SN_SOCK_H_

#include "netaddr.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Represents a socket
 * */
typedef struct sn_sock_t_ sn_sock_t;

int sn_sock_init(sn_sock_t* ss, int type);

int sn_sock_init_binded(sn_sock_t* ss, sn_netaddr_t* na);

int sn_sock_destroy(sn_sock_t* ss);

int sn_sock_bind(sn_sock_t* ss, sn_netaddr_t* na);

ssize_t sn_sock_send(sn_sock_t* ss, const void* buf, size_t len, int flags, const sn_netaddr_t* dst_addr);

ssize_t sn_sock_recv(sn_sock_t* ss, void* buf, size_t len, int flags, sn_netaddr_t* src_addr);

struct sn_sock_t_ {
    int fd;
    int type;
    int binded;
    sn_netaddr_t bind;
};

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif/*SN_SOCK_H_*/
