#include <stdlib.h>
#include <endian.h>
#include <string.h>
#include "proto.h"

// text component ?
// text component json ?
// entity metadata ?
// slot ?
// slot hashed ?
// nbt ?
// everything else ?

int proto_len_varint(int32_t value) {
	if (value < 0) return 5;
	for (int i = 1; i < 5; i++) {
		if (value < 1UL << (i * 7)) return i;
	}
	return 5;
}

int proto_len_varlong(int64_t value) {
	if (value < 0) return 10;
	for (int i = 1; i < 10; i++) {
		if (value < 1ULL << (i * 7)) return i;
	}
	return 10;
}

//////////////////////////////////////////

size_t proto_enc_bool(struct arraybuf *buf, uint8_t value) {
	uint8_t *write = arraybuf_write(buf, sizeof(value));
	if (write == NULL) return 0;
	*write = value != 0;
	return sizeof(value);
}

size_t proto_enc_byte(struct arraybuf *buf, int8_t value) {
	int8_t *write = arraybuf_write(buf, sizeof(value));
	if (write == NULL) return 0;
	*write = value != 0;
	return sizeof(value);
}

size_t proto_enc_ubyte(struct arraybuf *buf, uint8_t value) {
	uint8_t *write = arraybuf_write(buf, sizeof(value));
	if (write == NULL) return 0;
	*write = value != 0;
	return sizeof(value);
}

size_t proto_enc_short(struct arraybuf *buf, int16_t value) {
	int16_t *write = arraybuf_write(buf, sizeof(value));
	if (write == NULL) return 0;
	*write = value != 0;
	return sizeof(value);
}

size_t proto_enc_ushort(struct arraybuf *buf, uint16_t value) {
	uint16_t *write = arraybuf_write(buf, sizeof(value));
	if (write == NULL) return 0;
	*write = value != 0;
	return sizeof(value);
}

size_t proto_enc_int(struct arraybuf *buf, int32_t value) {
	int32_t *write = arraybuf_write(buf, sizeof(value));
	if (write == NULL) return 0;
	*write = value != 0;
	return sizeof(value);
}

size_t proto_enc_long(struct arraybuf *buf, int64_t value) {
	int64_t *write = arraybuf_write(buf, sizeof(value));
	if (write == NULL) return 0;
	*write = value != 0;
	return sizeof(value);
}

size_t proto_enc_varint(struct arraybuf *buf, int32_t value) {
	size_t vlen = proto_len_varint(value);
	uint8_t *write = arraybuf_write(buf, vlen);
	uint32_t v = (uint32_t)value;
	while (v & ~0x7fUL) {
		*write++ = (v & 0x7f) | 0x80;
		v >>= 7;
	}
	*write++ = v;
	return vlen;
}

size_t proto_enc_varlong(struct arraybuf *buf, int64_t value) {
	size_t vlen = proto_len_varlong(value);
	uint8_t *write = arraybuf_write(buf, vlen);
	uint64_t v = (uint64_t)value;
	while (v & ~0x7fUL) {
		*write++ = (v & 0x7f) | 0x80;
		v >>= 7;
	}
	*write++ = v;
	return vlen;
}

size_t proto_enc_bytes(struct arraybuf *buf, const uint8_t *value, size_t n) {
	uint8_t *write = arraybuf_write(buf, n);
	if (write == NULL) return 0;
	memcpy(write, value, n);
	return n;
}

size_t proto_enc_string(struct arraybuf *buf, const uint8_t *value, size_t n) {
	size_t vi_len = proto_enc_varint(buf, n);
	if (vi_len == 0) return 0;
	if (proto_enc_bytes(buf, value, n)) return 0;
	return vi_len + n;
}

size_t proto_enc_pos(struct arraybuf *buf, int x, int y, int z) {
	uint64_t v = 0;
	v |= ((uint64_t)x & 0x3ffffff) << 38;
	v |= ((uint64_t)z & 0x3ffffff) << 12;
	v |= ((uint64_t)y & 0x0000fff);
	uint64_t *write = arraybuf_write(buf, sizeof(v));
	*write = v;
	return sizeof(v);
}

size_t proto_enc_uuid(struct arraybuf *buf, UUID uuid) {
	uint64_t *write = arraybuf_write(buf, 16);
	memcpy(write, uuid, 16);
	return 16;
}

//////////////////////////////////////////

size_t proto_dec_bool(struct arraybuf *buf, uint8_t *out) {
	uint8_t *read = arraybuf_read(buf, sizeof(*out));
	if (!read) return 0;
	if (out) *out = *read;
	return sizeof(*out);
}

size_t proto_dec_byte(struct arraybuf *buf, int8_t *out) {
	int8_t *read = arraybuf_read(buf, sizeof(*out));
	if (!read) return 0;
	if (out) *out = *read;
	return sizeof(*out);
}

size_t proto_dec_ubyte(struct arraybuf *buf, uint8_t *out) {
	uint8_t *read = arraybuf_read(buf, sizeof(*out));
	if (!read) return 0;
	if (out) *out = *read;
	return sizeof(*out);
}

size_t proto_dec_short(struct arraybuf *buf, int16_t *out) {
	int16_t *read = arraybuf_read(buf, sizeof(*out));
	if (!read) return 0;
	if (out) *out = *read;
	return sizeof(*out);
}

size_t proto_dec_ushort(struct arraybuf *buf, uint16_t *out) {
	uint16_t *read = arraybuf_read(buf, sizeof(*out));
	if (!read) return 0;
	if (out) *out = *read;
	return sizeof(*out);
}

size_t proto_dec_int(struct arraybuf *buf, int32_t *out) {
	int32_t *read = arraybuf_read(buf, sizeof(*out));
	if (!read) return 0;
	if (out) *out = *read;
	return sizeof(*out);
}

size_t proto_dec_long(struct arraybuf *buf, int64_t *out) {
	int64_t *read = arraybuf_read(buf, sizeof(*out));
	if (!read) return 0;
	if (out) *out = *read;
	return sizeof(*out);
}

size_t proto_dec_varint(struct arraybuf *buf, int32_t *out) {
	uint32_t o = 0;
	uint8_t c;

	for (int i = 0; i < 5; i++) {
		if (arraybuf_read_one(buf, &c) != 1) return 0;
		o |= (c & 0x7f) << i * 7;

		if ((c & 0x80) == 0) {
			if (out) *out = o;
			return i + 1;
		}
	}

	return -1;
}

size_t proto_dec_varlong(struct arraybuf *buf, int64_t *out) {
	uint32_t o = 0;
	uint8_t c;

	for (int i = 0; i < 10; i++) {
		if (arraybuf_read_one(buf, &c) != 1) return 0;
		o |= (c & 0x7f) << i * 7;

		if ((c & 0x80) == 0) {
			if (out) *out = o;
			return i + 1;
		}
	}

	return -1;
}

size_t proto_dec_bytes(struct arraybuf *buf, uint8_t **out, size_t n) {
	uint8_t *read = arraybuf_read(buf, n);
	if (!read) return 0;
	if (out) *out = read;
	return n;
}

size_t proto_dec_string(struct arraybuf *buf, uint8_t **out, size_t *n) {
	int str_len;
	size_t vi_len = proto_dec_varint(buf, &str_len);
	if (vi_len < 1) return 0;
	if (proto_dec_bytes(buf, out, str_len) == 0) return 0;
	if (n) *n = str_len;
	return vi_len + *n;
}

size_t proto_dec_pos(struct arraybuf *buf, int *x, int *y, int *z) {
	uint64_t *read = arraybuf_read(buf, sizeof(uint64_t));
	if (!read) return 0;
	uint64_t v = *read;
	if (!x || !y || !z) return 8;
	*x = (v >> 38) & 0x03ffffff | (v & 1ULL << 63) ? 0xfc000000 : 0;
	*y = (v >> 12) & 0x03ffffff | (v & 1ULL << 37) ? 0xfc000000 : 0;
	*z = (v >> 0)  & 0x00000fff | (v & 1ULL << 11) ? 0xfffff000 : 0;
	return sizeof(uint64_t);
}

size_t proto_dec_uuid(struct arraybuf *buf, UUID out) {
	const char *read = arraybuf_read(buf, 16);
	if (!read) return 0;
	memcpy(out, read, 16);
	return 16;
}

