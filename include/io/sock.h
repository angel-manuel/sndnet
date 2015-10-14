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

/**
 * Invalid socket value
 * */
#define SN_IO_SOCK_INVALID -1

/**
 * Creates a socket binded to a name
 * @param name Netaddress that is the name the socket should have
 * @return A socket or SN_IO_SOCK_INVALID if ERROR
 * */
sn_io_sock_t sn_io_sock_named(const sn_io_naddr_t* name);

/**
 * Close a socket. Keeps trying if socket closing failed
 * because of a signal interrupt but fails silently if failed because of some other thing.
 * @param socket Socket to be closed
 * */
void sn_io_sock_close(sn_io_sock_t socket);

/**
 * Gets the name of a socket, that is, the binding address of that socket.
 * @param socket Socket
 * @param[out] out_name Place to store the name
 * @return 0 if ok, -1 if error
 * */
int sn_io_sock_get_name(sn_io_sock_t socket, sn_io_naddr_t* out_name);

/**
 * Sends some data using a socket
 * @param socket Socket
 * @param buf Data buffer
 * @param len Data buffer length
 * @param dst Destination netaddress
 * @return Bytes sent or -1 if error
 * */
ssize_t sn_io_sock_send(sn_io_sock_t socket, const void* buf, size_t len, const sn_io_naddr_t* dst);

/**
 * Receives some data from a socket
 * @param socket Socket
 * @param[out] buf Data buffer
 * @param len Data buffer length
 * @param[out] src Place to store source netaddress. Can be NULL.
 * @return Bytes received or -1 if error.
 * */
ssize_t sn_io_sock_recv(sn_io_sock_t socket, void* buf, size_t len, sn_io_naddr_t* src);

/**
 * Receives some data from a socket but KEEPS it on the queue for subsequent readings.
 * @param socket Socket
 * @param[out] buf Data buffer
 * @param len Data buffer length
 * @param[out] src Place to store source netaddress. Can be NULL.
 * @return Bytes received or -1 if error.
 * */
ssize_t sn_io_sock_peek(sn_io_sock_t socket, void* buf, size_t len, sn_io_naddr_t* src);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif/*SN_IO_SOCK_H_*/
