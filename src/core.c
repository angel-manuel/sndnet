#include "core.h"

#include "common.h"
#include "net/entry.h"
#include "net/packet.h"
#include "wire.h"

#include <assert.h>
#include <stdint.h>

int sn_core_deliver(sn_state_t* state, const sn_net_packet_t* packet, const sn_io_naddr_t* rem_addr) {
    assert(state != NULL);
    assert(packet != NULL);

    SN_UNUSED(rem_addr);

    return 0;
}

int sn_core_forward(sn_state_t* state, sn_net_packet_t* packet, const sn_io_naddr_t* rem_addr, sn_net_entry_t* nexthop) {
    int sent;

    assert(state != NULL);
    assert(packet != NULL);
    assert(nexthop != NULL);

    SN_UNUSED(rem_addr);

    if(!packet->header.ttl) {
        return -1;
    }

    packet->header.ttl--;

    sent = sn_net_packet_send(packet, state->socket, &(nexthop->net_addr));

    if(sent == -1) {
        packet->header.ttl++;
        return -1;
    }

    return 0;
}
