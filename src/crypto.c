#include <stddef.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/bn.h>
#include "crypto.h"

#define POLY 0x82F63B78

uint32_t crypto_crc32c_table[256];

void crypto_crc32c_init(void) {
    for (uint32_t i = 0; i < 256; i++) {
        uint32_t crc = i;
        for (int j = 0; j < 8; j++) {
            if (crc & 1) {
                crc = (crc >> 1) ^ POLY;
            } else {
                crc >>= 1;
            }
        }
        crypto_crc32c_table[i] = crc;
    }
}

uint32_t crypto_crc32c(const uint8_t *data, size_t length) {
    uint32_t crc = 0xFFFFFFFF; 
    for (size_t i = 0; i < length; i++) {
        uint8_t index = (crc ^ data[i]) & 0xFF;
        crc = (crc >> 8) ^ crypto_crc32c_table[index];
    }
    return crc ^ 0xFFFFFFFF; 
}

int crypto_gen_keypair() {
	private_key = EVP_RSA_gen(1024);
	if (!private_key) return -1;

	pub_key_len = i2d_PUBKEY(pkey, &pub_key_der);
	return (*pub_key_len > 0) - 1;
}

