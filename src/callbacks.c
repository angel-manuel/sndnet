#include "callbacks.h"

#include "common.h"
#include "net/addr.h"
#include "net/packet.h"
#include "net/router.h"
#include "io/naddr.h"
#include "util/closure.h"
#include "node.h"

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

void sn_silent_log_callback(int argc, void* argv[]) {
    SN_UNUSED(argc);
    SN_UNUSED(argv);
}

void sn_named_log_callback(int argc, void* argv[]) {
    assert(argc >= 2);
    assert(argv[0] != NULL);
    assert(argv[1] != NULL);

    fprintf(stderr, "%s: %s\n", (char*)argv[0], (char*)argv[1]);
}
