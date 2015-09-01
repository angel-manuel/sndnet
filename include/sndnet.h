#ifndef SNDNET_SNDNET_H_
#define SNDNET_SNDNET_H_

/**
 * Initialization of a node
 * @param[in] port UDP listening port
 * @param[out] sns Initialized state(must be already allocated)
 * @return 0 if OK, -1 otherwise
 * */
int sndnet_init(short port, SNState* sns);

/**
 * Destroyes a node
 * @param sns State to be destroyed(but not deallocated)
 * */
void sndnet_destroy(SNState* sns);

#endif/*SNDNET_SNDNET_H_*/
