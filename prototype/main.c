#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "address.h"
#include "message.h"
#include "sndnet.h"

int main() {
    sndnet_state_t sns;
    sndnet_addr_t self;
    char line[1024];
    char* command;

    sndnet_address_from_hex(&self, "abcd");
    
    sndnet_init(&sns, &self, 7777);
    
    while(printf("> "), fgets(line, 1024, stdin)) {
        command = strtok(line, " \n");

        if(!command)
            continue;
        
        if(strcmp(command, "quit") == 0) {
            break;
        }

        if(strcmp(command, "send") == 0) {
            char *dst, *payload;
            sndnet_addr_t sn_dst;
            
            dst = strtok(0, " \n");

            if(!dst) {
                printf("send <dst> <src> <payload>\n");
                continue;
            }

            sndnet_address_from_hex(&sn_dst, dst);

            payload = strtok(0, "\n");

            if(!payload) {
                printf("send <dst> <payload>\n");
                continue;
            }

            sndnet_send(&sns, &sn_dst, strlen(payload), payload);
        }

        if(strcmp(command, "insert") == 0) {
            char *addr;
            sndnet_addr_t sn_addr;

            addr = strtok(0, " \n");

            if(!addr) {
                printf("insert <addr>\n");
                continue;
            }

            sndnet_address_from_hex(&sn_addr, addr);

            sndnet_router_add(&(sns.router), &sn_addr, 0);
        }
    }
    
    sndnet_destroy(&sns);
    
    return 0;
}
