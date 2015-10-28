#include "net/packet.h"

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

sn_net_packet_t* sn_net_packet_recv(sn_io_sock_t socket, sn_io_naddr_t* src_addr) {
    sn_net_packet_t* packet;
    int recv_count;
    size_t packet_size;
    uint16_t payload_len = 0;

    assert(socket != SN_IO_SOCK_INVALID);

    recv_count = sn_io_sock_peek(socket, &payload_len, sizeof(payload_len), src_addr);

    if(recv_count < (ssize_t)sizeof(payload_len)) {
        if(recv_count < 0) {
            return NULL;
        } else {
            sn_io_sock_recv(socket, NULL, 0, NULL);
            return NULL;
        }
    }

    if(payload_len > SN_NET_PACKET_MAX_LEN) {
        sn_io_sock_recv(socket, NULL, 0, NULL);
        return NULL;
    }

    packet_size = sizeof(sn_net_packet_t) + (size_t)payload_len + 1;

    packet = (sn_net_packet_t*)malloc(packet_size);

    if(!packet) {
        return NULL;
    }

    recv_count = sn_io_sock_recv(socket, &packet->header, packet_size, NULL);

    if(recv_count < (ssize_t)sizeof(sn_wire_net_header_t) + (ssize_t)payload_len) {
        if(recv_count < 0) {
            return NULL;
        } else {
            free(packet);
            return NULL;
        }
    }

    packet->payload[payload_len] = '\0';

    return packet;
}

sn_net_packet_t* sn_net_packet_pack(const sn_net_addr_t* dst, const sn_net_addr_t* src, uint8_t type, size_t len, const char* payload) {
    sn_net_packet_t* packet;

    assert(dst != NULL);
    assert(src != NULL);
    assert(payload != NULL || len == 0);

    packet = (sn_net_packet_t*)malloc(sizeof(sn_net_packet_t) + len + 1);

    if(!packet)
        return 0;

    packet->header.ttl = SN_NET_PACKET_DEFAULT_TTL;
    packet->header.type = type;
    packet->header.len = len;

    sn_net_addr_ser(src, &packet->header.src);
    sn_net_addr_ser(dst, &packet->header.dst);

    memcpy(packet->payload, payload, len);
    packet->payload[len] = '\0';

    return packet;
}

void sn_net_packet_sign(sn_net_packet_t* packet, const sn_crypto_sign_key_t* key) {
    assert(packet != NULL);
    assert(key != NULL);

    sn_crypto_sign(
        key,
        (unsigned char*)&packet->header.dst,
        packet->header.len + (sizeof(sn_wire_net_header_t) - offsetof(sn_wire_net_header_t, dst)),
        &packet->header.sign);
}

int sn_net_packet_check_sign(const sn_net_packet_t* packet) {
    assert(packet != NULL);

    return sn_crypto_sign_check(
        &packet->header.sign,
        (sn_crypto_sign_pubkey_t*)&packet->header.src,
        (unsigned char*)&packet->header.dst,
        packet->header.len + (sizeof(sn_wire_net_header_t) - offsetof(sn_wire_net_header_t, dst))
    );
}

int sn_net_packet_send(const sn_net_packet_t* packet, sn_io_sock_t socket, const sn_io_naddr_t* dst_addr) {
    ssize_t packet_size;
    ssize_t sent;

    assert(packet != NULL);
    assert(socket != SN_IO_SOCK_INVALID);
    assert(dst_addr != NULL);

    packet_size = sizeof(sn_wire_net_header_t) + (size_t)packet->header.len;

    sent = sn_io_sock_send(socket, packet, packet_size, dst_addr);

    if(sent < packet_size)
        return -1;

    return 0;
}

void sn_net_packet_get_dst(const sn_net_packet_t* packet, sn_net_addr_t* out_dst) {
    assert(packet != NULL);
    assert(out_dst != NULL);

    sn_net_addr_deser(out_dst, &packet->header.dst);
}

void sn_net_packet_get_src(const sn_net_packet_t* packet, sn_net_addr_t* out_src) {
    assert(packet != NULL);
    assert(out_src != NULL);

    sn_net_addr_deser(out_src, &packet->header.src);
}

void sn_net_packet_header_to_str(const sn_net_packet_t* packet, char* out_str) {
    char dst_str[SN_NET_ADDR_PRINTABLE_LEN];
    char src_str[SN_NET_ADDR_PRINTABLE_LEN];

    assert(packet != NULL);
    assert(out_str != NULL);

    sn_net_addr_to_str((sn_net_addr_t*)&packet->header.dst, dst_str);
    sn_net_addr_to_str((sn_net_addr_t*)&packet->header.src, src_str);

    snprintf(out_str, SN_NET_PACKET_PRINTABLE_LEN,
    "dst:%s\n"
    "src:%s\n"
    "ttl:%hhu\n"
    "type:%hhu\n"
    "len:%hu\n",
    dst_str, src_str, packet->header.ttl, packet->header.type, packet->header.len);
}
