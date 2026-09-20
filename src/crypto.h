#pragma once
#include <stdint.h>
#include <openssl/evp.h>

extern EVP_PKEY *private_key;
extern uint8_t *pub_key_der;
extern size_t pub_key_len;

void crypto_crc32c_init(void);

uint32_t crypto_crc32c(const uint8_t *data, size_t length);

int crypto_gen_keypair();

