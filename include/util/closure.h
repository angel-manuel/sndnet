/**
* @file
* Implements a callback ADT
* */

#ifndef SN_UTIL_CLOSURE_H_
#define SN_UTIL_CLOSURE_H_

#ifdef __cplusplus
extern "C" {
#endif

#define SN_UTIL_CLOSURE_MAX_CURRYING 8

typedef void (*sn_util_closure_body_t)(int argc, void* argv[]);

typedef struct sn_util_closure_t_ sn_util_closure_t;

void sn_util_closure_init(sn_util_closure_t* closure, sn_util_closure_body_t body);
int sn_util_closure_init_curried(sn_util_closure_t* closure, sn_util_closure_body_t body, int argc, void* argv[]);
int sn_util_closure_init_curried_once(sn_util_closure_t* closure, sn_util_closure_body_t body, void* arg);
void sn_util_closure_call(sn_util_closure_t* closure, int argc, void* argv[]);
int sn_util_closure_curry(sn_util_closure_t* closure, int argc, void* argv[]);
int sn_util_closure_curry_once(sn_util_closure_t* closure, void* arg);

struct sn_util_closure_t_ {
    int c_argc;
    void* c_argv[SN_UTIL_CLOSURE_MAX_CURRYING];
    sn_util_closure_body_t body;
};

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif/*SN_UTIL_CLOSURE_H_*/
