#include "sndnet.h"

#include <stdio.h>

#include <tinycthread.h>

int main() {
	SNState sns;
	
	sndnet_init(&sns, 7777);
	
	thrd_join(sns.bg_thrd, NULL);
	
	sndnet_destroy(&sns);
	
	return 0;
}
