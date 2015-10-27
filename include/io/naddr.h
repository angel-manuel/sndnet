/**
 * @file
 * Implements an network address ADT
 * */

#ifndef SN_IO_NADDR_H_
#define SN_IO_NADDR_H_

#include "../common.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Represents a socket address/endpoint/name
 * */
typedef struct sockaddr sn_io_naddr_t;

/**
 * Maximum acceptable path length
 * */
#define SN_IO_NADDR_MAX_PATH_LEN 14

/**
 * IO Netaddress string representation length
 * */
#define SN_IO_NADDR_PRINTABLE_LEN (5 + SN_MAX(16 + 6, SN_IO_NADDR_MAX_PATH_LEN) + 1)

/**
 * Initializes an address from an ipv4 dot notation string and a port
 * @param addr Netaddress to be initialized
 * @param ipv4 IPv4 in dot notation (6.4.55.234)
 * @param port Port
 * @return 0 if OK, -1 if ERROR
 * */
int sn_io_naddr_ipv4(sn_io_naddr_t* addr, const char* ipv4, uint16_t port);

/**
 * Initializes an address from a path. Creates an unix domain socket address or a name for a named pipe.
 * @param addr Netaddress to be initialized
 * @param path Path. If starts with '_', hidden addresses will be tried
 * @return 0 if OK, -1 if ERROR
 * */
int sn_io_naddr_local(sn_io_naddr_t* addr, const char* path);

/**
 * Initializes an address from a str.
 * @param addr Netaddress
 * @param str String representation of a netaddress (INET:123.45.80.97:7890) (UNIX:)
 * @return 0 if OK, -1 if ERROR
 * */
int sn_io_naddr_from_str(sn_io_naddr_t* addr, const char* str);

/**
 * Passes an address to its string representation
 * @param addr Netaddress
 * @param[out] out_str String representation of the netaddress
 * @return 0 if OK, -1 if ERROR
 * */
int sn_io_naddr_to_str(const sn_io_naddr_t* addr, char* out_str);

/**
 * Compares the addresses
 * @param A A
 * @param B B
 * @return A - B
 **/
int sn_io_naddr_cmp(const sn_io_naddr_t* A, const sn_io_naddr_t* B);

typedef struct sn_io_naddr_ser_t_ {
    uint8_t type;
    union {
        struct {
            uint32_t ipv4;
            uint16_t port;
        } inet;
        char local_path[SN_IO_NADDR_MAX_PATH_LEN];
    };
} sn_io_naddr_ser_t;

/**
 * Serializes a netaddress
 * @param addr Netaddress
 * @param[out] ser Serialized netaddress
 * @return 0 if OK, -1 if ERROR
 * */
int sn_io_naddr_ser(const sn_io_naddr_t* addr, sn_io_naddr_ser_t* ser);

/**
 * Deserializes a netaddress
 * @param[out] addr Netaddress
 * @param ser Serialized netaddress
 * @return 0 if OK, -1 if ERROR
 * */
int sn_io_naddr_deser(sn_io_naddr_t* addr, const sn_io_naddr_ser_t* ser);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif/*SN_IO_NADDR_H_*/
