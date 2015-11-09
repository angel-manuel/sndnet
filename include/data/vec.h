#ifndef SN_DATA_VEC_H_
#define SN_DATA_VEC_H_

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct sn_data_vec_t_ sn_data_vec_t;

int sn_data_vec_init(sn_data_vec_t* vec, size_t elem_size);
int sn_data_vec_with_capacity(sn_data_vec_t* vec, size_t elem_size, size_t capacity);
void sn_data_vec_destroy(sn_data_vec_t* vec);
size_t sn_data_vec_size(const sn_data_vec_t* vec);
int sn_data_vec_at(sn_data_vec_t* vec, size_t at, void* out);
int sn_data_vec_push(sn_data_vec_t* vec, void* elem);
int sn_data_vec_remove_at(sn_data_vec_t* vec, size_t at, void* out);

struct sn_data_vec_t_ {
    size_t elem_size; /**< Element size*/
    size_t size; /**< Vec size on elems */
    size_t capacity; /**< Vec capacity on elems */
    char* vec;
};

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif/*SN_DATA_VEC_H_*/
