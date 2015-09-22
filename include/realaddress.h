/**
 * @file
 * Provides functions for managing SO network addresses
 * */

#ifndef SNDNET_REALADDRESS_H_
#define SNDNET_REALADDRESS_H_

#include <sys/types.h>
#include <sys/socket.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * SecondNet address length
 * */
#define SNDNET_REALADDRESS_HOSTNAME_LENGTH 256

/**
 * Network address
 * */
typedef struct sndnet_realaddr_t_ sndnet_realaddr_t;

/**
 * SO representation of a netowrk address
 * */
typedef struct sockaddr sndnet_soaddr_t;

/**
 * Initializes an address from a hostname
 * @param snra sndnet_realaddr_t to be initialized
 * @param hostname Hostname
 * */
void sndnet_realaddress_from_hostname(sndnet_realaddr_t* snra, const char* hostname);

/**
 * Gets the Hostname
 * @param snra The address
 * @return Hostname
 * */
const unsigned char* sndnet_realaddress_get_hostname(const sndnet_addr_t* snra);

struct sndnet_realaddr_t_ {
    sndnet_soaddr_t soaddr;
    char hostname[SNDNET_REALADDRESS_HOSTNAME_LENGTH];
};

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif/*SNDNET_REALADDRESS_H_*/
