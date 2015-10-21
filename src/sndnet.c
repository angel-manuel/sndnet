#include "sndnet.h"

#include <sodium.h>

int sn_init() {
    /* Sodium */

    if(sodium_init() == -1)
        return -1;

    return 0;
}
