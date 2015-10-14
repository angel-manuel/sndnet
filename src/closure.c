#include "closure.h"

#include <assert.h>
#include <stddef.h>

void sn_closure_init(sn_closure_t* closure, sn_closure_body_t body) {
    assert(closure != NULL);
    assert(body != NULL);

    closure->body = body;
    closure->c_argc = 0;
}

int sn_closure_init_curried(sn_closure_t* closure, sn_closure_body_t body, int argc, void* argv[]) {
    sn_closure_init(closure, body);
    return sn_closure_curry(closure, argc, argv);
}

int sn_closure_init_curried_once(sn_closure_t* closure, sn_closure_body_t body, void* arg) {
    sn_closure_init(closure, body);
    return sn_closure_curry_once(closure, arg);
}

void sn_closure_call(sn_closure_t* closure, int argc, void* argv[]) {
    assert(closure != NULL);
    assert(argv != NULL || argc == 0);

    {
        int call_argc = closure->c_argc + argc;
        void* call_argv[call_argc];

        for(int i = 0; i < closure->c_argc; ++i)
            call_argv[i] = closure->c_argv[i];

        for(int i = 0; i < argc; ++i)
            call_argv[i + closure->c_argc] = argv[i];

        closure->body(call_argc, call_argv);
    }
}

int sn_closure_curry(sn_closure_t* closure, int argc, void* argv[]) {
    int orig_argc;

    assert(closure != NULL);
    assert(argv != NULL || argc == 0);

    orig_argc = closure->c_argc;

    for(int i = 0; i < argc; ++i) {
        if(sn_closure_curry_once(closure, argv[i]) == -1) {
            closure->c_argc = orig_argc;
            return -1;
        }
    }

    return 0;
}

int sn_closure_curry_once(sn_closure_t* closure, void* arg) {
    assert(closure != NULL);

    if(closure->c_argc >= SN_CLOSURE_MAX_CURRYING)
        return -1;

    closure->c_argv[closure->c_argc++] = arg;

    return 0;
}
