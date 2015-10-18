#include "../catch.hpp"

#include "crypto/sign.h"

#include <stdio.h>
#include <string.h>

TEST_CASE("Public-key signatures work", "[crypto_sign]") {
    REQUIRE(sodium_init() != -1);

    SECTION("Signing and checking") {
        sn_crypto_sign_pubkey_t pk;
        sn_crypto_sign_key_t sk;
        sn_crypto_sign_t sign;
        unsigned char msg[] = "Hello";

        sn_crypto_sign_keypair(&pk, &sk);
        sn_crypto_sign(&sk, msg, 6, &sign);

        REQUIRE(sn_crypto_sign_check(&sign, &pk, msg, 6) == 0);
    }

    SECTION("Naked nacl sign") {
        unsigned char pk[crypto_sign_PUBLICKEYBYTES];
        unsigned char sk[crypto_sign_SECRETKEYBYTES];
        unsigned char m[] = "Hello";
        unsigned char sm[sizeof(m)+crypto_sign_BYTES];
        unsigned char m2[sizeof(m)+crypto_sign_BYTES];
        unsigned long long smlen;
        unsigned long long m2len;

        crypto_sign_keypair(pk, sk);
        crypto_sign(sm, &smlen, m, sizeof(m), sk);
        REQUIRE(crypto_sign_open(m2, &m2len, sm, smlen, pk) == 0);
        REQUIRE(memcmp(m, m2, sizeof(m)) == 0);
    }
}
