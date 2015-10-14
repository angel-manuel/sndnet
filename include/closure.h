/**
* @file
* Implements a callback ADT
* */

#ifndef SN_CLOSURE_H_
#define SN_CLOSURE_H_

#ifdef __cplusplus
extern "C" {
#endif

#define SN_CLOSURE_MAX_CURRYING 8

typedef void (*sn_closure_body_t)(int argc, void* argv[]);

typedef struct sn_closure_t_ sn_closure_t;

void sn_closure_init(sn_closure_t* closure, sn_closure_body_t body);
int sn_closure_init_curried(sn_closure_t* closure, sn_closure_body_t body, int argc, void* argv[]);
int sn_closure_init_curried_once(sn_closure_t* closure, sn_closure_body_t body, void* arg);
void sn_closure_call(sn_closure_t* closure, int argc, void* argv[]);
int sn_closure_curry(sn_closure_t* closure, int argc, void* argv[]);
int sn_closure_curry_once(sn_closure_t* closure, void* arg);

struct sn_closure_t_ {
    int c_argc;
    void* c_argv[SN_CLOSURE_MAX_CURRYING];
    sn_closure_body_t body;
};

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif/*SN_CLOSURE_H_*/
