#include "crypto/sign.h"

#include <assert.h>
#include <string.h>
#include <stddef.h>

void sn_crypto_sign_keypair(sn_crypto_sign_pubkey_t *pk, sn_crypto_sign_key_t *sk) {
    assert(pk != NULL);
    assert(sk != NULL);

    crypto_sign_keypair(pk->pk, sk->sk);
}

void sn_crypto_sign(const sn_crypto_sign_key_t *sk, const unsigned char *m, unsigned long long mlen, sn_crypto_sign_t *out_sign) {
    assert(sk != NULL);
    assert(m != NULL);
    assert(out_sign != NULL);

    crypto_sign_detached(out_sign->signature, NULL, m, mlen, sk->sk);
}

int sn_crypto_sign_check(const sn_crypto_sign_t *sign, const sn_crypto_sign_pubkey_t *pk, const unsigned char *m, unsigned long long mlen) {
    assert(sign != NULL);
    assert(pk != NULL);
    assert(m != NULL);

    return crypto_sign_verify_detached(sign->signature, m, mlen, pk->pk);
}
