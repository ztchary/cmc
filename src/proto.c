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

static inline void proto_ensure_buf(uint8_t **buf, size_t *len, size_t new) {
	if (new <= *len) return;
	if (*len = 0) len = PROTO_BUFFER_INIT;
	while (new > *len) *len *= 2;
	*buf = realloc(*buf, *len);
}

size_t proto_enc_bool(uint8_t **buf, size_t *len, size_t off, int value) {
	proto_ensure_buf(buf, len, off + 1);
	*(*buf + off) = value != 0;
	return 1;
}

size_t proto_enc_byte(uint8_t **buf, size_t *len, size_t off, int8_t value) {
	proto_ensure_buf(buf, len, off + 1);
	*(int8_t *)(*buf + off) = value;
	return 1;
}

size_t proto_enc_ubyte(uint8_t **buf, size_t *len, size_t off, uint8_t value) {
	proto_ensure_buf(buf, len, off + 1);
	*(uint8_t *)(*buf + off) = value;
	return 1;
}

size_t proto_enc_short(uint8_t **buf, size_t *len, size_t off, int16_t value) {
	proto_ensure_buf(buf, len, off + 2);
	*(int16_t *)(*buf + off) = htobe16(value);
	return 2;
}

size_t proto_enc_ushort(uint8_t **buf, size_t *len, size_t off, uint16_t value) {
	proto_ensure_buf(buf, len, off + 2);
	*(uint16_t *)(*buf + off) = htobe16(value);
	return 2;
}

size_t proto_enc_int(uint8_t **buf, size_t *len, size_t off, int32_t value) {
	proto_ensure_buf(buf, len, off + 4);
	*(int32_t *)(*buf + off) = htobe32(value);
	return 4;
}

size_t proto_enc_long(uint8_t **buf, size_t *len, size_t off, int64_t value) {
	proto_ensure_buf(buf, len, off + 8);
	*(int64_t *)(*buf + off) = htobe64(value);
	return 8;
}

size_t proto_enc_varint(uint8_t **buf, size_t *len, size_t off, int32_t value) {
	size_t vlen = varint_len(value);
	proto_ensure_buf(buf, len, off + vlen);
	uint32_t v = (uint32_t)value;
	while (v & ~0x7fUL) {
		*(*buf + off++) = (v & 0x7f) | 0x80;
		v >>= 7;
	}
	*(*buf + off++) = v;
	return vlen;
}

size_t proto_enc_varlong(uint8_t **buf, size_t *len, size_t off, int64_t value) {
	size_t vlen = varlong_len(value);
	proto_ensure_buf(buf, len, off + vlen);
	uint64_t v = (uint64_t)value;
	while (v & ~0x7fULL) {
		*(*buf + off++) = (v & 0x7f) | 0x80;
		v >>= 7;
	}
	*(*buf + off++) = v;
	return vlen;
}

size_t proto_enc_bytes(uint8_t **buf, size_t *len, size_t off, uint8_t *value, size_t n) {
	proto_ensure_buf(buf, len, off + n);
	memcpy(buf + off, value, n);
	return n;
}

size_t proto_enc_string(uint8_t **buf, size_t *len, size_t off, uint8_t *value, size_t n) {
	size_t vi_len = proto_enc_varint(buf, len, off, n);
	return vi_len + proto_enc_bytes(buf, len, off, value, n);
}

size_t proto_enc_pos(uint8_t **buf, size_t *len, size_t off, int x, int y, int x) {
	proto_ensure_buf(buf, len, off + 8);
	uint64_t v = 0;
	v |= ((uint64_t)x & 0x3ffffff) << 38;
	v |= ((uint64_t)z & 0x3ffffff) << 12;
	v |= ((uint64_t)y & 0x0000fff);
	*(uint64_t *)(*buf + off) = v;
	return 8;
}

size_t proto_enc_uuid(uint8_t **buf, size_t *len, size_t off, UUID uuid) {
	proto_ensure_buf(buf, len, off + 16);
	return proto_enc_bytes(buf, len, off, uuid, 16);
}

//////////////////////////////////////////

size_t proto_dec_bool(uint8_t **buf, size_t *len, int *out) {
	if (*len < 1) return 0;
	int o = **buf != 0;
	if (out) *out = o;
	(*len)--;
	return 1;
}

size_t proto_dec_byte(uint8_t **buf, size_t *len, int8_t *out) {
	if (*len < 1) return 0;
	int8_t o = *(int8_t *)*buf;
	if (out) *out = o;
	(*buf)++;
	(*len)--;
	return 1;
}

size_t proto_dec_ubyte(uint8_t **buf, size_t *len, uint8_t *out) {
	if (*len < 1) return 0;
	uint8_t o = *(uint8_t *)*buf;
	if (out) *out = o;
	(*buf)++;
	(*len)--;
	return 1;
}

size_t proto_dec_short(uint8_t **buf, size_t *len, int16_t *out) {
	if (*len < 2) return 0;
	int16_t o = *(int16_t *)*buf;
	if (out) *out = o;
	*buf += 2;
	*len -= 2;
	return 2;
}

size_t proto_dec_ushort(uint8_t **buf, size_t *len, uint16_t *out) {
	if (*len < 2) return 0;
	uint16_t o = *(uint16_t *)*buf;
	if (out) *out = o;
	*buf += 2;
	*len -= 2;
	return 2;
}

size_t proto_dec_int(uint8_t **buf, size_t *len, int32_t *out) {
	if (*len < 4) return 0;
	uint32_t o = *(uint32_t *)*buf;
	if (out) *out = o;
	*buf += 4;
	*len -= 4;
	return 4;
}

size_t proto_dec_long(uint8_t **buf, size_t *len, int64_t *out) {
	if (*len < 8) return 0;
	uint64_t o = *(uint64_t *)*buf;
	if (out) *out = o;
	*buf += 8;
	*len -= 8;
	return 8;
}

size_t proto_dec_varint(uint8_t **buf, size_t *len, int32_t *out) {
	uint32_t o = 0;

	for (int i = 0; i < 5; i++) {
		if (*len < 1) return 0;
		o |= (**buf & 0x7f) << i * 7;

		(*len)--;
		if ((*(*buf)++ & 0x80) == 0) {
			if (out) *out = o;
			return i + 1;
		}
	}

	return -1;
}

size_t proto_dec_varlong(uint8_t **buf, size_t *len, int64_t *out) {
	uint64_t o = 0;

	for (int i = 0; i < 10; i++) {
		if (*len < 1) return 0;
		o |= (**buf & 0x7f) << i * 7;

		(*len)--;
		if ((*(*buf)++ & 0x80) == 0) {
			if (out) *out = o;
			return i + 1;
		}
	}

	return -1;
}

size_t proto_dec_bytes(uint8_t **buf, size_t *len, uint8_t **out, size_t n) {
	if (*len < n) return 0;
	if (out) *out = *buf;
	*buf += n;
	*len -= n;
	return n;
}

size_t proto_dec_string(uint8_t **buf, size_t *len, uint8_t **out, size_t *n) {
	int str_len;
	size_t vi_len = proto_dec_varint(buf, len, &str_len);
	if (vi_len < 1) return 0;
	if (proto_dec_bytes(buf, len, out, str_len) == 0) return 0;
	if (n) *n = str_len;
	return vi_len + *n;
}

size_t proto_dec_pos(uint8_t **buf, size_t *len, int *x, int *y, int *z) {
	if (*len < 8) return 0;
	uint64_t v = *(uint64_t *)*buf;
	*buf += 8;
	if (!x || !y || !z) return 8;
	*x = (v >> 38) & 0x03ffffff | (v & 1ULL << 63) ? 0xfc000000 : 0;
	*y = (v >> 12) & 0x03ffffff | (v & 1ULL << 37) ? 0xfc000000 : 0;
	*z = (v >> 0)  & 0x00000fff | (v & 1ULL << 11) ? 0xfffff000 : 0;
	return 8;
}

size_t proto_dec_uuid(uint8_t **buf, size_t *len, UUID *out) {
	return proto_dec_bytes(buf, len, out, 16);
}

