/**
 * @file
 * Provides functions for working with sockets
 * */

#ifndef SN_IO_SOCK_H_
#define SN_IO_SOCK_H_

#include "naddr.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Represents a socket
 * */
typedef int sn_io_sock_t;

#define SN_IO_SOCK_INVALID -1

sn_io_sock_t sn_io_sock_named(const sn_io_naddr_t* name);
void sn_io_sock_close(sn_io_sock_t socket);
int sn_io_sock_get_name(sn_io_sock_t socket, sn_io_naddr_t* out_name);
ssize_t sn_io_sock_send(sn_io_sock_t socket, const void* buf, size_t len, const sn_io_naddr_t* dst);
ssize_t sn_io_sock_recv(sn_io_sock_t socket, void* buf, size_t len, sn_io_naddr_t* src);
ssize_t sn_io_sock_peek(sn_io_sock_t socket, void* buf, size_t len, sn_io_naddr_t* src);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif/*SN_IO_SOCK_H_*/
