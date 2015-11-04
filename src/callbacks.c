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

void sn_default_log_callback(int argc, void* argv[]) {
    assert(argc >= 2);
    assert(argv[1] != NULL);

    fprintf(stderr, "sndnet: %s\n", (char*)argv[1]);
}

void sn_default_forward_callback(int argc, void* argv[]) {
    char nh_addr[SN_NET_ADDR_PRINTABLE_LEN];
    char nh_raddr[SN_IO_NADDR_PRINTABLE_LEN];
    char packet_str[SN_NET_PACKET_PRINTABLE_LEN];
    char rem_addr_str[SN_IO_NADDR_PRINTABLE_LEN];
    sn_net_packet_t* packet = argv[1];
    sn_node_t* sns = argv[2];
    sn_io_naddr_t* rem_addr = argv[3];
    sn_net_entry_t* nexthop = argv[4];

    assert(argc >= 5);
    assert(packet != NULL);
    assert(sns != NULL);
    assert(nexthop != NULL);

    sn_net_addr_to_str(&nexthop->addr, nh_addr);
    sn_io_naddr_to_str(&nexthop->net_addr, nh_raddr);
    sn_net_packet_header_to_str(packet, packet_str);

    if(rem_addr != NULL)
        sn_io_naddr_to_str(rem_addr, rem_addr_str);
    else
        strncpy(rem_addr_str, "THIS", SN_IO_NADDR_PRINTABLE_LEN);

    if(!packet->header.ttl) {
        sn_node_log(sns,
        "packet without TTL\n"
        "sent from %s\n"
        "%s"
        "Would forwarding to %s @ %s\n",
        rem_addr_str, packet_str, nh_addr, nh_raddr);
        return;
    }

    packet->header.ttl--;

    if(sn_net_packet_send(packet, sns->socket, &nexthop->net_addr) == -1) {
        packet->header.ttl++;
        sn_node_log(sns, "ERROR sending packet to: %s\n", rem_addr_str);
    } else {
        sn_node_log(sns,
        "packet forward\n"
        "sent from %s\n"
        "%s"
        "Forwarding to %s @ %s\n",
        rem_addr_str, packet_str, nh_addr, nh_raddr);
    }
}

void sn_default_deliver_callback(int argc, void* argv[]) {
    char packet_str[SN_NET_PACKET_PRINTABLE_LEN];
    const sn_net_packet_t* packet = argv[1];
    sn_node_t* sns = argv[2];
    sn_io_naddr_t* rem_addr = argv[3];

    assert(argc >= 4);
    assert(packet != NULL);
    assert(sns != NULL);

    sn_net_packet_header_to_str(packet, packet_str);

    if(rem_addr == NULL)
        sn_node_log(sns,
            "packet deliver\n"
            "sent from THIS\n"
            "%s"
            "\n%s\n",
        packet_str, packet->payload);
    else {
        char rem_addr_str[SN_IO_NADDR_PRINTABLE_LEN];

        sn_io_naddr_to_str(rem_addr, rem_addr_str);

        sn_node_log(sns,
            "packet deliver\n"
            "sent from %s\n"
            "%s"
            "\n%s\n",
        rem_addr_str, packet_str, packet->payload);
    }
}
