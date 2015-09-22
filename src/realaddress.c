#include "realaddress.h"

#include <assert.h>
#include <netdb.h>

void sndnet_realaddress_from_hostname(sndnet_realaddr_t* snra, const char* hostname) {
    assert(snra != 0);
    assert(hostname != 0);

    //TODO: getaddrinfo()
}

const unsigned char* sndnet_realaddress_get_hostname(const sndnet_addr_t* snra) {

}
