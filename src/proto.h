#pragma once
#include <stddef.h>
#include <stdint.h>
#include "data.h"

#define PROTO_BUFFER_INIT 256
#define PROTO_PACKET_MAX  2097151
#define PROTO_INFLATE_MAX 8388608

typedef uint8_t *UUID;

int proto_len_varint(int32_t);
int proto_len_varlong(int64_t);

size_t proto_enc_bool(struct arraybuf *, uint8_t);
size_t proto_enc_byte(struct arraybuf *, int8_t);
size_t proto_enc_ubyte(struct arraybuf *, uint8_t);
size_t proto_enc_short(struct arraybuf *, int16_t);
size_t proto_enc_ushort(struct arraybuf *, uint16_t);
size_t proto_enc_int(struct arraybuf *, int32_t);
size_t proto_enc_long(struct arraybuf *, int64_t);
size_t proto_enc_varint(struct arraybuf *, int32_t);
size_t proto_enc_varlong(struct arraybuf *, int64_t);
size_t proto_enc_bytes(struct arraybuf *, const uint8_t *, size_t);
size_t proto_enc_string(struct arraybuf *, const uint8_t *, size_t);
size_t proto_enc_pos(struct arraybuf *, int, int, int);
size_t proto_enc_uuid(struct arraybuf *, UUID);

size_t proto_dec_bool(struct arraybuf *, uint8_t *);
size_t proto_dec_byte(struct arraybuf *, int8_t *);
size_t proto_dec_ubyte(struct arraybuf *, uint8_t *);
size_t proto_dec_short(struct arraybuf *, int16_t *);
size_t proto_dec_ushort(struct arraybuf *, uint16_t *);
size_t proto_dec_int(struct arraybuf *, int32_t *);
size_t proto_dec_long(struct arraybuf *, int64_t *);
size_t proto_dec_varint(struct arraybuf *, int32_t *);
size_t proto_dec_varlong(struct arraybuf *, int64_t *);
size_t proto_dec_bytes(struct arraybuf *, uint8_t **, size_t);
size_t proto_dec_string(struct arraybuf *, uint8_t **, size_t *);
size_t proto_dec_pos(struct arraybuf *, int *, int *, int *);
size_t proto_dec_uuid(struct arraybuf *, UUID);

