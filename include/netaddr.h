/**
* @file
* Implements en ADT capable of representing addresses of different families
* */

#ifndef SN_NETADDR_H_
#define SN_NETADDR_H_

#include "localaddr.h"
#include "realaddr.h"

#include <stdint.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MAX(A, B) (((A) > (B)) ? (A) : (B))

#define SN_NETADDR_PRINTABLE_LEN (6 + MAX(SN_REALADDR_PRINTABLE_LEN, SN_LOCALADDR_PRINTABLE_LEN))

typedef struct sockaddr sn_netaddr_t;

int sn_netaddr_bind(const sn_netaddr_t* na, int fd);

socklen_t sn_netaddr_get_size(const sn_netaddr_t* na);

int sn_netaddr_cmp(const sn_netaddr_t* a, const sn_netaddr_t* b);

int sn_netaddr_tostr(const sn_netaddr_t* na, char* out_str);

typedef struct sn_netaddr_ser_t_ {
    int32_t type;
    union {
        sn_realaddr_ser_t realaddr;
        sn_localaddr_ser_t localaddr;
    };
} sn_netaddr_ser_t;

int sn_netaddr_ser(const sn_netaddr_t* na, sn_netaddr_ser_t* ser);
int sn_netaddr_deser(sn_netaddr_t* na, const sn_netaddr_ser_t* ser);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif/*SN_NETADDR_H_*/
