#pragma once
#include <stddef.h>
#include <stdint.h>

#define PROTO_BUFFER_INIT 256
#define PROTO_PACKET_MAX  2097151
#define PROTO_INFLATE_MAX 8388608

typedef uint8_t *UUID;

int proto_len_varint(int32_t);
int proto_len_varlong(int64_t);

size_t proto_enc_bool(uint8_t, size_t, size_t, int);
size_t proto_enc_byte(uint8_t, size_t, size_t, int8_t);
size_t proto_enc_ubyte(uint8_t, size_t, size_t, uint8_t);
size_t proto_enc_short(uint8_t, size_t, size_t, int16_t);
size_t proto_enc_ushort(uint8_t, size_t, size_t, uint16_t);
size_t proto_enc_int(uint8_t, size_t, size_t, int32_t);
size_t proto_enc_long(uint8_t, size_t, size_t, int64_t);
size_t proto_enc_varint(uint8_t, size_t, size_t, int32_t);
size_t proto_enc_varlong(uint8_t, size_t, size_t, int64_t);
size_t proto_enc_bytes(uint8_t, size_t, size_t, uint8_t, size_t);
size_t proto_enc_string(uint8_t, size_t, size_t, uint8_t, size_t);
size_t proto_enc_pos(uint8_t, size_t, size_t, int, int, int);
size_t proto_enc_uuid(uint8_t, size_t, size_t, UUID);

size_t proto_dec_bool(uint8_t, size_t, int);
size_t proto_dec_byte(uint8_t, size_t, int8_t);
size_t proto_dec_ubyte(uint8_t, size_t, uint8_t);
size_t proto_dec_short(uint8_t, size_t, int16_t);
size_t proto_dec_ushort(uint8_t, size_t, uint16_t);
size_t proto_dec_int(uint8_t, size_t, int32_t);
size_t proto_dec_long(uint8_t, size_t, int64_t);
size_t proto_dec_varint(uint8_t, size_t, int32_t);
size_t proto_dec_varlong(uint8_t, size_t, int64_t);
size_t proto_dec_bytes(uint8_t, size_t, uint8_t, size_t);
size_t proto_dec_string(uint8_t, size_t, uint8_t, size_t);
size_t proto_dec_pos(uint8_t, size_t, int, int, int);
size_t proto_dec_uuid(uint8_t, size_t, UUID);
