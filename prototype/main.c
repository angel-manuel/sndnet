#include "sndnet.h"

#include <pthread.h>
#include <stdio.h>
#include <string.h>

int main() {
    SNState sns;
    char line[1024];
    char* command;
    
    sndnet_init(&sns, 7777);
    
    while(fgets(line, 1024, stdin)) {
        command = strtok(line, " \n");
        
        if(strcmp(command, "quit") == 0) {
            break;
        }
    }
    
    sndnet_destroy(&sns);
    
    return 0;
}
