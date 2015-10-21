/**
 * @file
 * SecondNet main header
 * */

#ifndef SN_SN_H_
#define SN_SN_H_

#include "node.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initializes common SecondNet data. Thread safe, can be called multiple times.
 * @return 0 if correctly initialized, 1 if was already initialized, -1 if error
 * */
int sn_init();

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif/*SN_SN_H_*/
