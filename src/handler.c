#include "handler.h"

#include "node.h"
#include "net/entry.h"
#include "net/packet.h"
#include "io/naddr.h"
#include "wire.h"
#include "common.h"

#include <assert.h>

//Forward handlers declarations

const sn_forward_handler_t sn_default_forward_handlers[] = {
    NULL,
    NULL,
    NULL
};

SN_ASSERT_COMPILE(sizeof(sn_default_forward_handlers) == SN_WIRE_NET_TYPES*sizeof(sn_forward_handler_t));

//Deliver handlers declarations

int deliver_user_handler(sn_node_t* sns, const sn_net_packet_t* packet, const sn_io_naddr_t* rem_addr);
int deliver_reply_handler(sn_node_t* sns, const sn_net_packet_t* packet, const sn_io_naddr_t* rem_addr);
int deliver_ping_handler(sn_node_t* sns, const sn_net_packet_t* packet, const sn_io_naddr_t* rem_addr);

const sn_deliver_handler_t sn_default_deliver_handlers[] = {
    deliver_user_handler,
    deliver_reply_handler,
    deliver_ping_handler
};

SN_ASSERT_COMPILE(sizeof(sn_default_deliver_handlers) == SN_WIRE_NET_TYPES*sizeof(sn_deliver_handler_t));

//Forward handlers definitions

//Deliver handlers definitions

int deliver_user_handler(sn_node_t* sns, const sn_net_packet_t* packet, const sn_io_naddr_t* rem_addr) {
    assert(sns != NULL);
    assert(packet != NULL);

    SN_UNUSED(rem_addr);

    return sn_node_upcall(sns, packet->payload, packet->header.len);
}

int deliver_reply_handler(sn_node_t* sns, const sn_net_packet_t* packet, const sn_io_naddr_t* rem_addr) {
    assert(sns != NULL);
    assert(packet != NULL);

    SN_UNUSED(rem_addr);

    assert(0);

    return 1;
}

int deliver_ping_handler(sn_node_t* sns, const sn_net_packet_t* packet, const sn_io_naddr_t* rem_addr) {
    const sn_wire_ping_msg_t* ping = (sn_wire_ping_msg_t*)packet->payload;
    sn_net_addr_t src;

    assert(sns != NULL);
    assert(packet != NULL);

    SN_UNUSED(rem_addr);

    sn_net_packet_get_src(packet, &src);

    return sn_node_send_typed(sns, &src, SN_WIRE_NET_TYPE_REPLY, sizeof(*ping), (const char*)ping);
}

