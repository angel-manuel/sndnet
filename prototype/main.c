#include <pthread.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "net/addr.h"
#include "net/packet.h"
#include "io/naddr.h"
#include "net/router.h"
#include "sndnet.h"

sn_node_t sns;

int main(int argc, char* argv[]) {
    sn_crypto_sign_pubkey_t pk;
    sn_crypto_sign_key_t sk;
    char self_addr_str[SN_NET_ADDR_PRINTABLE_LEN];
    uint16_t port;
    char line[1024];
    char* command;

    if(argc < 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        return 1;
    }

    if(sscanf(argv[1], "%hu", &port) < 1)
        return -1;

    if(sn_init() == -1)
        return -1;

    sn_crypto_sign_keypair(&pk, &sk);

    if(sn_node_at_port(&sns, &sk, &pk, port, 1) == -1) {
        fprintf(stderr, "Error initializing\n");
        return 1;
    }

    sn_net_addr_to_str((sn_net_addr_t*)&pk, self_addr_str);

    printf("Initialized @ %s\n", self_addr_str);

    while(printf("> "), fgets(line, 1024, stdin)) {
        command = strtok(line, " \n");

        if(!command)
            continue;

        if(strcmp(command, "quit") == 0) {
            break;
        }

        if(strcmp(command, "send") == 0) {
            char *dst, *payload;
            sn_net_addr_t sn_dst;

            dst = strtok(0, " \n");

            if(!dst) {
                printf("send <dst> <src> <payload>\n");
                continue;
            }

            sn_net_addr_from_hex(&sn_dst, dst);

            payload = strtok(0, "\n");

            if(!payload) {
                printf("send <dst> <payload>\n");
                continue;
            }

            sn_node_send(&sns, &sn_dst, strlen(payload), payload);
        }

        if(strcmp(command, "insert") == 0) {
            char *addr, *raddr;
            sn_net_addr_t sn_net_addr;
            sn_io_naddr_t sn_raddr;

            addr = strtok(0, " \n");

            if(!addr) {
                printf("insert <sn_net_addr> <addr(type:ip:port)>\n");
                continue;
            }

            raddr = strtok(0, " \n");

            if(!raddr) {
                printf("insert <sn_net_addr> <addr(type:ip:port)>\n");
                continue;
            }

            sn_net_addr_from_hex(&sn_net_addr, addr);
            sn_io_naddr_from_str(&sn_raddr, raddr);

            sn_net_router_add(&(sns.router), &sn_net_addr, &sn_raddr);
        }

        if(strcmp(command, "show") == 0) {
            char* buffer = malloc(30000);

            if(!buffer)
                return 1;

            sn_net_router_to_str(&sns.router, buffer, 30000);

            printf("%s\n", buffer);

            free(buffer);
        }
    }

    sn_node_destroy(&sns);

    return 0;
}
