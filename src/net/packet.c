#include "net/packet.h"

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

sn_net_packet_t* sn_net_packet_recv(sn_io_sock_t socket, sn_io_naddr_t* src_addr) {
    sn_wire_net_header_t header;
    sn_net_packet_t* msg;
    int recv_count;
    uint16_t size;

    assert(socket != SN_IO_SOCK_INVALID);

    memset(&header, 0, sizeof(header));

    recv_count = sn_io_sock_peek(socket, &header, sizeof(header), src_addr);

    if(recv_count < (ssize_t)sizeof(header)) {
        if(recv_count < 0) {
            return 0;
        } else {
            sn_io_sock_recv(socket, NULL, 0, NULL);
            return 0;
        }
    }

    if(header.len > SN_NET_PACKET_MAX_LEN) {
        sn_io_sock_recv(socket, NULL, 0, NULL);
        return 0;
    }

    size = sizeof(sn_net_packet_t) + (size_t)header.len + 1;

    msg = (sn_net_packet_t*)malloc(size);

    if(!msg) {
        return 0;
    }

    recv_count = sn_io_sock_recv(socket, &msg->header, size, NULL);

    if(recv_count < (ssize_t)sizeof(sn_wire_net_header_t) + (ssize_t)header.len) {
        if(recv_count < 0) {
            return 0;
        } else {
            free(msg);
            return 0;
        }
    }

    msg->payload[header.len] = '\0';

    return msg;
}

sn_net_packet_t* sn_net_packet_pack(const sn_net_addr_t* dst, const sn_net_addr_t* src, size_t len, const char* payload) {
    sn_net_packet_t* msg;

    assert(dst != NULL);
    assert(src != NULL);
    assert(payload != NULL || len == 0);

    msg = (sn_net_packet_t*)malloc(sizeof(sn_net_packet_t) + len + 1);

    if(!msg)
        return 0;

    sn_net_addr_ser(dst, &msg->header.dst);
    sn_net_addr_ser(src, &msg->header.src);
    msg->header.ttl = SN_NET_PACKET_DEFAULT_TTL;
    msg->header.len = len;
    memcpy(msg->payload, payload, len);
    msg->payload[len] = '\0';

    return msg;
}

int sn_net_packet_send(const sn_net_packet_t* msg, sn_io_sock_t socket, const sn_io_naddr_t* dst_addr) {
    ssize_t packet_size;
    ssize_t sent;

    assert(msg != NULL);
    assert(socket != SN_IO_SOCK_INVALID);
    assert(dst_addr != NULL);

    packet_size = sizeof(sn_wire_net_header_t) + (size_t)msg->header.len;

    sent = sn_io_sock_send(socket, msg, packet_size, dst_addr);

    if(sent < packet_size)
        return -1;

    return 0;
}

void sn_net_packet_get_dst(const sn_net_packet_t* msg, sn_net_addr_t* out_dst) {
    assert(msg != NULL);
    assert(out_dst != NULL);

    sn_net_addr_deser(out_dst, &msg->header.dst);
}

void sn_net_packet_get_src(const sn_net_packet_t* msg, sn_net_addr_t* out_src) {
    assert(msg != NULL);
    assert(out_src != NULL);

    sn_net_addr_deser(out_src, &msg->header.src);
}

void sn_net_packet_header_to_str(const sn_net_packet_t* msg, char* out_str) {
    char dst_str[SN_NET_ADDR_PRINTABLE_LEN];
    char src_str[SN_NET_ADDR_PRINTABLE_LEN];

    assert(msg != NULL);
    assert(out_str != NULL);

    sn_net_addr_to_str((sn_net_addr_t*)&msg->header.dst, dst_str);
    sn_net_addr_to_str((sn_net_addr_t*)&msg->header.src, src_str);

    snprintf(out_str, SN_NET_PACKET_PRINTABLE_LEN,
    "dst:%s\n"
    "src:%s\n"
    "ttl:%hhu\n"
    "len:%hhu\n",
    dst_str, src_str, msg->header.ttl, msg->header.len);
}