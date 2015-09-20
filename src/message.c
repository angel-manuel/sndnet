#include "message.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

SNMessage* sndnet_message_recv(int socket_fd) {
    SNHeader header;
    SNMessage* msg;
    struct sockaddr rem_addr;
    socklen_t addrlen = sizeof(rem_addr);
    int recv_count;
    uint16_t size;

    assert(socket_fd >= 0);

    memset(&header, 0, sizeof(header));

    recv_count = recvfrom(socket_fd, &header, sizeof(header), MSG_PEEK, &rem_addr, &addrlen);

    if(recv_count < sizeof(header)) {
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

    size = sizeof(SNMessage) + (size_t)header.len + 1;

    msg = (SNMessage*)malloc(size);

    if(!msg) {
        return 0;
    }

    recv_count = recvfrom(socket_fd, &(msg->header), size, 0, 0, 0);

    if(recv_count < sizeof(SNHeader) + (size_t)header.len) {
        if(recv_count < 0) {
            return 0;
        } else {
            free(msg);
            return 0;
        }
    }

    msg->payload[header.len] = '\0';
    memcpy(&(msg->rem_addr), &rem_addr, sizeof(rem_addr));

    return msg;
}