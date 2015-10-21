/**
 * @file
 * Implements predefined callbacks
 * */

#ifndef SN_CALLBACKS_H_
#define SN_CALLBACKS_H_

#ifdef __cplusplus
extern "C" {
#endif

void sn_silent_log_callback(int argc, void* argv[]);
void sn_named_log_callback(int argc, void* argv[]);
void sn_default_log_callback(int argc, void* argv[]);
void sn_default_forward_callback(int argc, void* argv[]);
void sn_default_deliver_callback(int argc, void* argv[]);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif/*SN_CALLBACKS_H_*/
