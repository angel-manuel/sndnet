/**
 * @file
 * Provides functions for managing SO network addresses
 * */

#ifndef SNDNET_REALADDRESS_H_
#define SNDNET_REALADDRESS_H_

#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * SO representation of a netowrk address
 * */
typedef struct sockaddr sndnet_soaddr_t;

/**
 * Network address
 * */
typedef sndnet_soaddr_t sndnet_realaddr_t;

/**
 * Initializes an address from a hostname
 * @param snra sndnet_realaddr_t to be initialized
 * @param hostname Hostname
 * @return 0 if OK, -1 if ERROR
 * */
int sndnet_realaddress_from_hostname(sndnet_realaddr_t* snra, const char* hostname, uint16_t port);

int sndnet_realaddress_get_hostname(const sndnet_realaddr_t* snra, char* out_hostname, size_t out_hostname_len);

int sndnet_realaddress_get_port(const sndnet_realaddr_t* snra, uint16_t* out_port);

int sndnet_realaddress_tostr(const sndnet_realaddr_t* snra, char* out_str, size_t out_str_len);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif/*SNDNET_REALADDRESS_H_*/
