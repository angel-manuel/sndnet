#include "core.h"

#include "common.h"
#include "net/entry.h"
#include "net/packet.h"
#include "node.h"
#include "wire.h"

#include <assert.h>
#include <stdint.h>

int sn_core_deliver(sn_node_t* state, const sn_net_packet_t* packet, const sn_io_naddr_t* rem_addr) {
    assert(state != NULL);
    assert(packet != NULL);

    SN_UNUSED(rem_addr);

    switch (packet->header.type) {
        case SN_WIRE_NET_TYPE_USER: /*User msg*/
            return sn_node_upcall(state, packet->payload, packet->header.len);
    }

    return -1;
}

int sn_core_forward(sn_node_t* state, sn_net_packet_t* packet, const sn_io_naddr_t* rem_addr, sn_net_entry_t* nexthop) {
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
