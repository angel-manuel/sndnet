/**
* @file
* Implements en ADT representing local addresses(for Unix Domain Sockets or Windows Named Pipes)
* */

#ifndef SN_LOCALADDR_H_
#define SN_LOCALADDR_H_

#include <assert.h>
#include <sys/socket.h>
#include <sys/un.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SN_LOCALADDR_MAX_PATH_LENGTH 14

#define SN_LOCALADDR_PRINTABLE_LEN SN_LOCALADDR_MAX_PATH_LENGTH

#define SN_LOCALADDR_HIDDEN

typedef struct sockaddr_un sn_localaddr_t;

void sn_localaddr_init(sn_localaddr_t* la, const char path[SN_LOCALADDR_MAX_PATH_LENGTH]);

int sn_localaddr_bind(const sn_localaddr_t* la, int fd);

int sn_localaddr_cmp(const sn_localaddr_t* a, const sn_localaddr_t* b);

void sn_localaddr_tostr(const sn_localaddr_t* la, char* out_str);

typedef struct sn_localaddr_ser_t_ {
    char path[SN_LOCALADDR_MAX_PATH_LENGTH];
} sn_localaddr_ser_t;

int sn_localaddr_ser(const sn_localaddr_t* la, sn_localaddr_ser_t* ser);
int sn_localaddr_deser(sn_localaddr_t* la, const sn_localaddr_ser_t* ser);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif/*SN_LOCALADDR_H_*/
