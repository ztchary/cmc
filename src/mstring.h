#include <stdint.h>

#define MSTRING_NEW_CAP 256

struct mstring {
	size_t cap;
	size_t len;
	char data[];
}

uint8_t *mstring_new();
size_t mstring_len(uint8_t *);
uint8_t *mstring_end(uint8_t *);
uint8_t *mstring_grow(uint8_t *, size_t);

