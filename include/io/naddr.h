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
#define SN_IO_NADDR_MAX_PATH_LEN 13

#define SN_IO_NADDR_PRINTABLE_LEN (5 + SN_MAX(16 + 6, SN_IO_NADDR_MAX_PATH_LEN) + 1)

int sn_io_naddr_ipv4(sn_io_naddr_t* addr, const char* ipv4, uint16_t port);
int sn_io_naddr_local(sn_io_naddr_t* addr, const char* path);
int sn_io_naddr_from_str(sn_io_naddr_t* addr, const char* str);
int sn_io_naddr_to_str(const sn_io_naddr_t* addr, char* out_str);

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

int sn_io_naddr_ser(const sn_io_naddr_t* addr, sn_io_naddr_ser_t* ser);
int sn_io_naddr_deser(sn_io_naddr_t* addr, const sn_io_naddr_ser_t* ser);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif/*SN_IO_NADDR_H_*/
