#pragma once
#include <stdint.h>
#include <openssl/evp.h>

EVP_PKEY *private_key;
uint8_t *public_key;
size_t public_key_len;

void crypto_crc32c_init(void);

uint32_t crypto_crc32c(const uint8_t *data, size_t length);

int crypto_gen_keypair();

