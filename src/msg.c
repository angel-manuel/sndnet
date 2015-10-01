#include "msg.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

sn_msg_t* sn_msg_recv(int socket_fd, sn_realaddr_t* rem_addr) {
    sn_header_t header;
    sn_msg_t* msg;
    socklen_t addrlen = sizeof(sn_realaddr_t);
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

    if(header.len > SN_MSG_MAX_LEN) {
        recvfrom(socket_fd, &header, sizeof(header), 0, 0, 0);
        return 0;
    }

    size = sizeof(sn_msg_t) + (size_t)header.len + 1;

    msg = (sn_msg_t*)malloc(size);

    if(!msg) {
        return 0;
    }

    recv_count = recvfrom(socket_fd, &(msg->header), size, 0, 0, 0);

    if(recv_count < (ssize_t)sizeof(sn_header_t) + (ssize_t)header.len) {
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

sn_msg_t* sn_msg_pack(const sn_addr_t* dst, const sn_addr_t* src, sn_msg_type_t type, size_t len, const char* payload) {
    sn_msg_t* msg;

    assert(dst != 0);
    assert(src != 0);
    assert(payload != 0 || len == 0);

    msg = (sn_msg_t*)malloc(sizeof(sn_msg_t) + len + 1);

    if(!msg)
        return 0;

    sn_addr_get_raw(dst, msg->header.dst);
    sn_addr_get_raw(src, msg->header.src);
    msg->header.ttl = SN_MSG_DEFAULT_TTL;
    msg->header.type = type;
    msg->header.len = len;
    memcpy(msg->payload, payload, len);
    msg->payload[len] = '\0';

    return msg;
}

int sn_msg_send(const sn_msg_t* msg, int socket_fd, const sn_realaddr_t* rem_addr) {
    size_t packet_size;
    ssize_t sent;

    assert(msg != 0);
    assert(socket_fd >= 0);
    assert(rem_addr != 0);

    packet_size = sizeof(sn_header_t) + (size_t)msg->header.len;

    sent = sendto(socket_fd, msg, packet_size, 0, rem_addr, sizeof(sn_realaddr_t));

    return sent;
}
