#include "localaddr.h"

#include <string.h>

void sn_localaddr_init(sn_localaddr_t* la, const char path[SN_LOCALADDR_MAX_PATH_LENGTH]) {
    assert(la != 0);
    assert(path != 0);

    la->sun_family = AF_UNIX;

    memset(la->sun_path, 0, SN_LOCALADDR_MAX_PATH_LENGTH);

    #ifdef SN_LOCALADDR_HIDDEN
    strncpy(la->sun_path+1, path, SN_LOCALADDR_MAX_PATH_LENGTH-1);
    la->sun_path[0] = '\0';
    #else
    strncpy(la->sun_path, path, SN_LOCALADDR_MAX_PATH_LENGTH);
    #endif

    la->sun_path[SN_LOCALADDR_MAX_PATH_LENGTH-1] = '\0';
}

int sn_localaddr_bind(const sn_localaddr_t* la, int fd) {
    assert(la != 0);
    assert(fd >= 0);

    return bind(fd, (struct sockaddr*)la, 2 + SN_LOCALADDR_MAX_PATH_LENGTH);
}

int sn_localaddr_cmp(const sn_localaddr_t* a, const sn_localaddr_t* b) {
    #ifdef SN_LOCALADDR_HIDDEN
    return strcmp(a->sun_path+1, b->sun_path+1);
    #else
    return strcmp(a->sun_path, b->sun_path);
    #endif
}

void sn_localaddr_tostr(const sn_localaddr_t* la, char* out_str) {
    assert(la != 0);
    assert(out_str != 0);

    #ifdef SN_LOCALADDR_HIDDEN
    out_str[0] = '_';
    strncpy(out_str+1, la->sun_path+1, SN_LOCALADDR_PRINTABLE_LEN-1);
    #else
    strncpy(out_str, la->sun_path, SN_LOCALADDR_PRINTABLE_LEN);
    #endif
}

int sn_localaddr_ser(const sn_localaddr_t* la, sn_localaddr_ser_t* ser) {
    assert(la != 0);
    assert(ser != 0);

    memcpy(ser->path, la->sun_path, SN_LOCALADDR_MAX_PATH_LENGTH);

    return 0;
}

int sn_localaddr_deser(sn_localaddr_t* la, const sn_localaddr_ser_t* ser) {
    assert(la != 0);
    assert(ser != 0);

    memcpy(la->sun_path, ser->path, SN_LOCALADDR_MAX_PATH_LENGTH);

    return 0;
}
