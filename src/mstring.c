#include <stdlib.h>
#include <string.h>
#include "mstring.h"

uint8_t *mstring_new() {
	struct mstring *mstr = malloc(MSTRING_NEW_CAP + sizeof(struct mstring));
	mstr->cap = MSTRING_NEW_CAP;
	mstr->len = 0;
	return mstr->data;
}

size_t mstring_len(uint8_t *str) {
	struct mstring *mstr = (struct mstring *)(str - 8);
	return mstr->len;
}

uint8_t *mstring_end(uint8_t *str) {
	struct mstring *mstr = (struct mstring *)(str - 8);
	return mstr->data + mstr->len;
}

uint8_t *mstring_grow(uint8_t *str, size_t grow) {
	struct mstring *mstr = (struct mstring *)(str - 8);
	size_t size = mstr->len + grow;
	if (mstr->cap < size) {
		while (mstr->cap < size) mstr->cap *= 2;
		mstr = realloc(mstr, mstr->cap + sizeof(struct mstring));
	}
	return mstr->data;
}
