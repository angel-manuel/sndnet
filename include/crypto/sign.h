#ifndef SN_CRYPTO_SIGN_H_
#define SN_CRYPTO_SIGN_H_

#include <sodium.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct sn_crypto_sign_t_ sn_crypto_sign_t;

typedef struct sn_crypto_sign_pubkey_t_ sn_crypto_sign_pubkey_t;

typedef struct sn_crypto_sign_key_t_ sn_crypto_sign_key_t;

void sn_crypto_sign_keypair(sn_crypto_sign_pubkey_t *pk, sn_crypto_sign_key_t *sk);

void sn_crypto_sign(sn_crypto_sign_key_t *sk, unsigned char *m, unsigned long long mlen, sn_crypto_sign_t *out_sign);

int sn_crypto_sign_check(sn_crypto_sign_t *sign, sn_crypto_sign_pubkey_t *pk, unsigned char *m, unsigned long long mlen);

struct sn_crypto_sign_pubkey_t_ {
    unsigned char pk[crypto_sign_PUBLICKEYBYTES];
};

struct sn_crypto_sign_key_t_ {
    unsigned char sk[crypto_sign_SECRETKEYBYTES];
};

struct sn_crypto_sign_t_ {
    unsigned char signature[crypto_sign_BYTES];
};

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif/*SN_CRYPTO_SIGN_H_*/
