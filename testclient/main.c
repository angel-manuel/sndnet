#include "sndnet.h"

#include <pthread.h>
#include <stdio.h>

int main() {
	SNState sns;
	
	sndnet_init(&sns, 7777);
	
	pthread_join(sns.bg_thrd, 0);
	
	sndnet_destroy(&sns);
	
	return 0;
}
