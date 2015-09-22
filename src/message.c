#include "message.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

sndnet_message_t* sndnet_message_recv(int socket_fd, sndnet_realaddr_t* rem_addr) {
    sndnet_header_t header;
    sndnet_message_t* msg;
    socklen_t addrlen = sizeof(sndnet_realaddr_t);
    int recv_count;
    uint16_t size;

    assert(socket_fd >= 0);
    assert(rem_addr != 0);

    memset(&header, 0, sizeof(header));

    recv_count = recvfrom(socket_fd, &header, sizeof(header), MSG_PEEK, rem_addr, &addrlen);

    if(recv_count < (ssize_t)sizeof(header)) {
        if(recv_count < 0) {
            return 0;
        } else {
            recvfrom(socket_fd, &header, sizeof(header), 0, 0, 0);
            return 0;
        }
    }

    if(header.len > SNDNET_MESSAGE_MAX_LENGTH) {
        recvfrom(socket_fd, &header, sizeof(header), 0, 0, 0);
        return 0;
    }

    size = sizeof(sndnet_message_t) + (size_t)header.len + 1;

    msg = (sndnet_message_t*)malloc(size);

    if(!msg) {
        return 0;
    }

    recv_count = recvfrom(socket_fd, &(msg->header), size, 0, 0, 0);

    if(recv_count < (ssize_t)sizeof(sndnet_header_t) + (ssize_t)header.len) {
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

sndnet_message_t* sndnet_message_pack(const sndnet_addr_t* dst, const sndnet_addr_t* src, size_t len, const char* payload) {
    sndnet_message_t* msg;

    assert(dst != 0);
    assert(src != 0);
    assert(payload != 0 || len == 0);

    msg = (sndnet_message_t*)malloc(sizeof(sndnet_message_t) + len + 1);

    if(!msg)
        return 0;

    memcpy(msg->header.dst, sndnet_address_get(dst), SNDNET_ADDRESS_LENGTH);
    memcpy(msg->header.src, sndnet_address_get(src), SNDNET_ADDRESS_LENGTH);
    msg->header.ttl = SNDNET_MESSAGE_DEFAULT_TTL;
    msg->header.len = len;
    memcpy(msg->payload, payload, len);
    msg->payload[len] = '\0';

    return msg;
}

int sndnet_message_send(const sndnet_message_t* msg, int socket_fd, const sndnet_realaddr_t* rem_addr) {
    size_t packet_size;
    ssize_t sent;

    assert(msg != 0);
    assert(socket_fd >= 0);
    assert(rem_addr != 0);

    packet_size = sizeof(sndnet_header_t) + (size_t)msg->header.len;

    sent = sendto(socket_fd, msg, packet_size, 0, rem_addr, sizeof(sndnet_realaddr_t));

    return sent;
}
