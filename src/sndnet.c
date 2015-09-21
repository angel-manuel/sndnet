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
#include "message.h"

void sndnet_deliver(sndnet_state_t* sns, const sndnet_message_t* msg);
int sndnet_forward(sndnet_state_t* sns, const sndnet_message_t* msg);
void sndnet_log(sndnet_state_t* sns, const char* format, ...);
void* sndnet_background(void* arg);

void default_log_cb(const char* msg);
void default_forward_cb(const sndnet_message_t* msg, sndnet_state_t* sns, sndnet_entry_t* nexthop);
void default_deliver_cb(const sndnet_message_t* msg, sndnet_state_t* sns);

int sndnet_init(sndnet_state_t* sns, const sndnet_addr_t* self, unsigned short port) {
    int socket_fd;
    struct sockaddr_in serv_addr;
    
    assert(sns != 0);
    
    /* Copying */
    
    sndnet_address_copy(&(sns->self), self);
    sndnet_router_init(&(sns->router), &(sns->self));
    sns->log_cb = default_log_cb;
    sns->deliver_cb = default_deliver_cb;
    sns->forward_cb = default_forward_cb;
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

void sndnet_destroy(sndnet_state_t* sns) {
    assert(sns != 0);
    
    sndnet_log(sns, "Destroying");
    
    /* Socket closing */
    
    close(sns->socket_fd);
    
    /* Thread closing */
    
    pthread_cancel(sns->bg_thrd);
    pthread_join(sns->bg_thrd, 0);
    
    sndnet_log(sns, "Destroyed");
}

void sndnet_set_log_callback(sndnet_state_t* sns, sndnet_log_callback cb) {
    assert(sns != 0);
    
    if(cb)
        sns->log_cb = cb;
    else
        sns->log_cb = default_log_cb;
    
    sndnet_log(sns, "Log callback changed");
}

void sndnet_set_forward_callback(sndnet_state_t* sns, sndnet_forward_callback cb) {
    assert(sns != 0);

    if(cb)
        sns->forward_cb = cb;
    else
        sns->forward_cb = default_forward_cb;

    sndnet_log(sns, "Forwarding callback changed");
}

void sndnet_set_deliver_callback(sndnet_state_t* sns, sndnet_deliver_callback cb) {
    assert(sns != 0);

    if(cb)
        sns->deliver_cb = cb;
    else
        sns->deliver_cb = default_deliver_cb;

    sndnet_log(sns, "Delivering callback changed");
}

int sndnet_send(sndnet_state_t* sns, const sndnet_addr_t* dst, size_t len, const char* payload) {
    sndnet_message_t* msg;

    assert(sns != 0);
    assert(dst != 0);
    assert(payload != 0 || len == 0);

    msg = sndnet_message_pack(dst, &(sns->self), len, payload);

    if(!msg)
        return -1;

    sndnet_forward(sns, msg);

    free(msg);

    return 0;
}

/*Private functions*/

void sndnet_deliver(sndnet_state_t* sns, const sndnet_message_t* msg) {
    assert(sns != 0);
    assert(msg != 0);

    (sns->deliver_cb)(msg, sns);
}

int sndnet_forward(sndnet_state_t* sns, const sndnet_message_t* msg) {
    sndnet_addr_t dst;
    sndnet_entry_t nexthop;

    assert(sns != 0);
    assert(msg != 0);

    sndnet_address_init(&dst, msg->header.dst);

    sndnet_router_nexthop(&(sns->router), &dst, &nexthop);

    if(nexthop.is_set) {
        (sns->forward_cb)(msg, sns, &nexthop);
        return 0;
        /*return sndnet_message_send(msg, sns->socket_fd, &(nexthop.net_addr));*/
    } else {
        sndnet_deliver(sns, msg);
        return 0;
    }
}

void sndnet_log(sndnet_state_t* sns, const char* format, ...) {
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
    sndnet_state_t* sns = (sndnet_state_t*)arg;
    sndnet_realaddr_t rem_addr;
    sndnet_message_t* msg;
    
    assert(sns != 0);
    
    do {
        msg = sndnet_message_recv(sns->socket_fd, &rem_addr);

        if(!msg)
            continue;

        sndnet_forward(sns, msg);
        
        free(msg);
    } while(1);
    
    return sns;
}

/*Default callbacks*/


void default_log_cb(const char* msg) {
    fprintf(stderr, "sndnet: %s\n", msg);
}

void default_forward_cb(const sndnet_message_t* msg, sndnet_state_t* sns, sndnet_entry_t* nexthop) {
    sndnet_log(sns, "Should forward to %s\n", sndnet_address_tostr(&(nexthop->sn_addr)));
}

void default_deliver_cb(const sndnet_message_t* msg, sndnet_state_t* sns) {
    sndnet_addr_t dst, src;

    sndnet_address_init(&dst, msg->header.dst);
    sndnet_address_init(&src, msg->header.src);

    sndnet_log(sns,
        "msg\n"
        "dst = %s\n"
        "src = %s\n"
        "ttl = %hu\n"
        "len = %u\n"
        "buf = %s\n",
    sndnet_address_tostr(&dst),
    sndnet_address_tostr(&src),
    msg->header.ttl, msg->header.len, msg->payload);
}
