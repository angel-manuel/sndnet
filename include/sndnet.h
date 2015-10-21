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
 * Initialized common SecondNet data. Not thread safe. Must be called just once.
 * @return 0 if OK, -1 if ERR
 * */
int sn_init();

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif/*SN_SN_H_*/
