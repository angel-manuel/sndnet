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
 * @param port Port
 * @return 0 if OK, -1 if ERROR
 * */
int sndnet_realaddress_from_hostname(sndnet_realaddr_t* snra, const char* hostname, uint16_t port);

/**
 * Gets the hostname of an address
 * @param snra The address
 * @param[out] out_hostname Pointer to string where hostname should be stored
 * @param out_hostname_len Length of out_hostname. Wont be surpassed, instead function willl return -1. 16 is the recommended length.
 * @return 0 if OK, -1 if ERROR
 * */
int sndnet_realaddress_get_hostname(const sndnet_realaddr_t* snra, char* out_hostname, size_t out_hostname_len);

/**
 * Gets the port of an address
 * @param snra The address
 * @param[out] out_port Pointer to 16-bit unsigned integer where port should be stored
 * @return 0 if OK, -1 if ERROR
 * */
int sndnet_realaddress_get_port(const sndnet_realaddr_t* snra, uint16_t* out_port);

/**
 * Gets an string representation of the address(<ip>:<port> style)
 * @param snra The address
 * @param[out] out_str Pointer to string where representation shuld be stored
 * @param out_hostname_len Length of out_string. Wont be surpassed, instead function will return -1. 23 is the recommended length.
 * @return 0 if OK, -1 if ERROR
 * */
int sndnet_realaddress_tostr(const sndnet_realaddr_t* snra, char* out_str, size_t out_str_len);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif/*SNDNET_REALADDRESS_H_*/
