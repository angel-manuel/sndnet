#include "sndnet.h"

#include <sodium.h>
#define asm __asm
#include <mintomic/mintomic.h>

mint_atomic32_t initialized = { 0 };

int sn_init() {
    /* Sodium */

    mint_thread_fence_acquire();

    if(mint_load_32_relaxed(&initialized) != 0)
        return 1;

    if(mint_compare_exchange_strong_32_relaxed(&initialized, 0, 1) != 0)
        return 1;

    mint_thread_fence_release();

    if(sodium_init() == -1)
        return -1;

    return 0;
}
