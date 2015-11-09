#include "data/vec.h"

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#define SN_DATA_VEC_DEFAULT_CAPACITY 8

int sn_data_vec_init(sn_data_vec_t* vec, size_t elem_size) {
    return sn_data_vec_with_capacity(vec, elem_size, SN_DATA_VEC_DEFAULT_CAPACITY);
}

int sn_data_vec_with_capacity(sn_data_vec_t* vec, size_t elem_size, size_t capacity) {
    assert(vec != NULL);
    assert(elem_size != 0);

    vec->elem_size = elem_size;
    vec->capacity = capacity;
    vec->size = 0;
    vec->vec = (char*)malloc(elem_size*capacity);

    if(vec->vec == NULL)
        return -1;
    
    return 0;
}

void sn_data_vec_destroy(sn_data_vec_t* vec) {
    assert(vec != NULL);

    vec->elem_size = vec->capacity = vec->size = 0;

    if(vec->vec)
        free(vec->vec);
}

size_t sn_data_vec_size(const sn_data_vec_t* vec) {
    assert(vec != NULL);

    return vec->size;
}

int sn_data_vec_at(sn_data_vec_t* vec, size_t at, void* out) {
    assert(vec != NULL);
    assert(out != NULL);

    if(at < vec->size) {
        memcpy(out, &vec->vec[at*vec->elem_size], vec->elem_size);
        return 0;
    }
    
    return -1;
}

int sn_data_vec_push(sn_data_vec_t* vec, void* elem) {
    assert(vec != NULL);
    assert(elem != NULL);

    assert(vec->size <= vec->capacity);

    if(vec->size == vec->capacity) {
        char* new_vec;

        vec->capacity *= 2;

        if(vec->capacity == 0)
            vec->capacity = 1;

        new_vec = (char*)realloc(vec->vec, vec->elem_size*vec->capacity);

        if(new_vec == NULL)
            return -1;

        vec->vec = new_vec;
    }

    memcpy(&vec->vec[(vec->size++)*vec->elem_size], elem, vec->elem_size);

    return 0;
}

int sn_data_vec_remove_at(sn_data_vec_t* vec, size_t at, void* out) {
    assert(vec != NULL);
    
    if(at >= vec->size)
        return -1;

    if(out != NULL)
        memcpy(out, &vec->vec[at*vec->elem_size], vec->elem_size);

    --vec->size;

    if(vec->size != at)
        memcpy(&vec->vec[at*vec->elem_size], &vec->vec[vec->size*vec->elem_size], vec->elem_size);

    if(vec->capacity/2 >= SN_DATA_VEC_DEFAULT_CAPACITY && vec->size < vec->capacity/2) {
        char* new_vec;

        vec->capacity /= 2;

        new_vec = (char*)realloc(vec->vec, vec->elem_size*vec->capacity);

        if(new_vec != NULL)
            vec->vec = new_vec;
    }

    return 0;
}
