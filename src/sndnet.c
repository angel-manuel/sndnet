#include "sndnet.h"

#include <assert.h>
#include <arpa/inet.h>
#include <nacl/crypto_box.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include "address.h"

#define SNDNET_MAX_MSG_LEN 1000
#define SNDNET_TTL_DEFAULT 32

typedef struct SNMessage_ {
    unsigned char dst[SNDNET_ADDRESS_LENGTH];
    unsigned char src[SNDNET_ADDRESS_LENGTH];
    uint16_t ttl;
    uint16_t len;
} SNMessage;

void sndnet_log(SNState* sns, const char* format, ...);
void* sndnet_background(void* arg);

void default_log_cb(const char* msg) {
    fprintf(stderr, "sndnet: %s\n", msg);
}

int sndnet_init(SNState* sns, unsigned short port) {
    int socket_fd;
    struct sockaddr_in serv_addr;
    
    assert(sns != 0);
    
    /* Copying */
    
    sns->log_cb = default_log_cb;
    sns->port = port;
    
    sndnet_log(sns, "Initializing");
    
    /* Socket initialization */
    
    socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    
    if(socket_fd == -1) {
        sndnet_log(sns, "Error while initializing socket");
        return 1;
    }
    
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(port);
    
    if(bind(socket_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1) {
        sndnet_log(sns, "Error while initializing socket");
        close(socket_fd);
        return 1;
    }
    
    sns->socket_fd = socket_fd;
    
    /* Background thread initialization */
    
    if(pthread_create(&(sns->bg_thrd), 0, sndnet_background, sns)) {
        sndnet_log(sns, "Error while starting thread");
        close(socket_fd);
        return 1;
    }
    
    sndnet_log(sns, "Initialized");
    
    return 0;
}

void sndnet_destroy(SNState* sns) {
    assert(sns != 0);
    
    sndnet_log(sns, "Destroying");
    
    /* Socket closing */
    
    close(sns->socket_fd);
    
    /* Thread closing */
    
    pthread_cancel(sns->bg_thrd);
    pthread_join(sns->bg_thrd, 0);
    
    sndnet_log(sns, "Destroyed");
}

void sndnet_set_log_callback(SNState* sns, sndnet_log_callback cb) {
    assert(sns != 0);
    
    if(cb)
        sns->log_cb = cb;
    else
        sns->log_cb = default_log_cb;
    
    sndnet_log(sns, "Log callback changed");
}

int sndnet_send(const SNState* sns, const SNAddress* dst, size_t len, const char* payload) {
    char *buffer;
    SNMessage *msg;
    char *msgbuf;
    SNEntry nexthop;

    assert(sns != 0);
    assert(dst != 0);
    assert(msg != 0 || len == 0);

    if(len > SNDNET_MAX_MSG_LEN)
        return -1;

    buffer = (char*)malloc(sizeof(SNMessage) + len);

    if(!buffer)
        return -1;

    msg = (SNMessage*)buffer;
    msgbuf = (char*)(msg + 1);

    memcpy(&(msg->dst), sndnet_address_get(dst), SNDNET_ADDRESS_LENGTH);
    memcpy(&(msg->src), sndnet_address_get(&(sns->self)), SNDNET_ADDRESS_LENGTH);
    msg->ttl = SNDNET_TTL_DEFAULT;
    msg->len = len;

    memcpy(msgbuf, payload, len);

    sndnet_router_nexthop(&(sns->router), dst, &nexthop);

    if(nexthop.is_set) {
        //TODO: forward
    } else {
        //TODO: deliver
    }

    free(buffer);

    return 0;
}

/*Private functions*/

void sndnet_log(SNState* sns, const char* format, ...) {
    char str[1024];
    va_list args;
    
    assert(sns != 0);
    assert(sns->log_cb != 0);
    assert(format != 0);
    
    va_start(args, format);
    vsnprintf(str, 1024, format, args);
    va_end(args);
    
    sns->log_cb(str);
}

void* sndnet_background(void* arg) {
    SNState* sns = (SNState*)arg;
    SNMessage msg;
    struct sockaddr rem_addr;
    socklen_t addrlen = sizeof(rem_addr);
    int recv_count;
    SNAddress dst, src;
    unsigned char* buffer;
    uint16_t size;
    
    assert(sns != 0);
    
    do {
        memset(&msg, 0, sizeof(msg));
        
        recv_count = recvfrom(sns->socket_fd, &msg, sizeof(msg), MSG_PEEK, &rem_addr, &addrlen);
        
        if(recv_count < sizeof(msg)) {
            if(recv_count < 0) {
                sndnet_log(sns, "Error on recvfrom()");
                return 0;
            } else if(recv_count) {
                sndnet_log(sns, "Message too short");
            }
            recvfrom(sns->socket_fd, &msg, sizeof(msg), 0, 0, 0);
            continue;
        }
        
        if(msg.len > SNDNET_MAX_MSG_LEN) {
            sndnet_log(sns, "Message too large: %zu bytes", msg.len);
            recvfrom(sns->socket_fd, &msg, sizeof(msg), 0, 0, 0);
            continue;
        }
        
        size = sizeof(SNMessage) + (size_t)msg.len;
        
        sndnet_log(sns, "Allocating %zu bytes", size + 1);
        buffer = (unsigned char*)malloc(size + 1);
        
        if(!buffer) {
            sndnet_log(sns, "Error on malloc");
            return 0;
        }
        
        recv_count = recvfrom(sns->socket_fd, buffer, size, 0, 0, 0);
        
        if(recv_count < size) {
            if(recv_count < 0) {
                sndnet_log(sns, "Error on recvfrom()");
                return 0;
            } else if(recv_count) {
                sndnet_log(sns, "Message shorter than specified in header");
            }
            free(buffer);
            continue;
        }
        
        buffer[size] = '\0';
        
        sndnet_address_init(&dst, msg.dst);
        sndnet_address_init(&src, msg.src);
        
        sndnet_log(sns, "msg\n"
        "dst = %s\n"
        "src = %s\n"
        "ttl = %hu\n"
        "len = %u\n"
        "buf = %s\n",
        sndnet_address_tostr(&dst),
        sndnet_address_tostr(&src),
        msg.ttl, msg.len, buffer + sizeof(SNMessage));
        
        free(buffer);
    } while(recv_count > 0);
    
    return sns;
}
