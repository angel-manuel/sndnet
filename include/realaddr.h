/**
 * @file
 * Provides functions for managing SO network addresses
 * */

#ifndef SN_REALADDR_H_
#define SN_REALADDR_H_

#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * SO representation of a netowrk address
 * */
typedef struct sockaddr sn_soaddr_t;

/**
 * Network address
 * */
typedef sn_soaddr_t sn_realaddr_t;

/**
 * sn_realaddr_t hostname printable representation length.
 * */
#define SN_REALADDR_HOSTNAME_PRINTABLE_LEN 16

/**
 * sn_realaddr_t port printable representation length.
 * */
#define SN_REALADDR_PORT_PRINTABLE_LEN 5

/**
 * sn_realaddr_t printable representation length.
 * */
#define SN_REALADDR_PRINTABLE_LEN (SN_REALADDR_HOSTNAME_PRINTABLE_LEN+SN_REALADDR_PORT_PRINTABLE_LEN+1)

/**
 * Initializes an address from a hostname
 * @param snra sn_realaddr_t to be initialized
 * @param hostname Hostname, if NULL or "bind" the returned address will be able to be binded
 * @param port Port
 * @return 0 if OK, -1 if ERROR
 * */
int sn_realaddr_from_hostname(sn_realaddr_t* snra, const char* hostname, uint16_t port);

/**
 * Initializes an address from its string representation(<ip>:<port>)
 * @param snra sn_realaddr_t to be initialized
 * @param str String representation, if hostname is "bind" the returned address will be able to be binded
 * @return 0 if OK, -1 if ERROR
 * */
int sn_realaddr_from_str(sn_realaddr_t* snra, const char* str);

/**
 * Binds a socket to an address for listening
 * @param snra sn_realaddr_t capable of being binded
 * @param socket_fd Socket to bind
 * @return 0 if OK, -1 if ERROR
 * */
int sn_realaddr_bind(sn_realaddr_t* snra, int socket_fd);

/**
 * Gets the hostname of an address
 * @param snra The address
 * @param[out] out_hostname Pointer to string where hostname should be stored
 * @return 0 if OK, -1 if ERROR
 * */
int sn_realaddr_get_hostname(const sn_realaddr_t* snra, char* out_hostname);

/**
 * Gets the port of an address
 * @param snra The address
 * @param[out] out_port Pointer to 16-bit unsigned integer where port should be stored
 * @return 0 if OK, -1 if ERROR
 * */
int sn_realaddr_get_port(const sn_realaddr_t* snra, uint16_t* out_port);

/**
 * Gets an string representation of the address(<ip>:<port> style)
 * @param snra The address
 * @param[out] out_str Pointer to string where representation shuld be stored
 * @return 0 if OK, -1 if ERROR
 * */
int sn_realaddr_tostr(const sn_realaddr_t* snra, char* out_str);

/**
 * Compares two realaddresses
 * @param a A realaddr
 * @param b Another realaddr
 * @return ret<0 if a < b, ret>0 if a > b, ret==0 if a == b
 * */
int sn_realaddr_cmp(const sn_realaddr_t* a, const sn_realaddr_t* b);

/**
 * Serialized sn_realaddr_t
 * */
typedef struct sn_realaddr_ser_t_ {
    uint16_t port; /**< Port number */
    uint32_t ipv4; /**< IPv4 octects*/
} sn_realaddr_ser_t;

/**
 * Serializes a sn_realaddr
 * @param snra A sn_realaddr
 * @param ser Pointer to serialized sn_realaddr
 * @return 0 if OK, -1 if ERROR
 * */
int sn_realaddr_ser(const sn_realaddr_t* snra, sn_realaddr_ser_t* ser);

/**
 * Deserializes a serialized sn_realaddr
 * @param snra Desearialized sn_realaddr
 * @param ser Pointer to serialized sn_realaddr
 * @return 0 if OK, -1 if ERROR
 * */
int sn_realaddr_deser(sn_realaddr_t* snra, const sn_realaddr_ser_t* ser);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif/*SN_REALADDR_H_*/
