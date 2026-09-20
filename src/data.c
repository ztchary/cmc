#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "data.h"

struct ringbuf *ringbuf_new(void) {
	struct ringbuf *buf = calloc(1, sizeof(struct ringbuf));
	buf->data = malloc(RINGBUF_SIZE);
	return buf;
}

void ringbuf_free(struct ringbuf *buf) {
	free(buf->data);
	free(buf);
}

size_t ringbuf_avail(struct ringbuf *buf) {
	if (buf->head >= buf->tail) return buf->head - buf->tail;
	return buf->head + RINGBUF_SIZE - buf->tail;
}

size_t ringbuf_used(struct ringbuf *buf) {
	if (buf->tail >= buf->head) return buf->tail - buf->head;
	return buf->tail + RINGBUF_SIZE - buf->head;
}

void *ringbuf_read(struct ringbuf *buf, size_t n) {
	if (n > ringbuf_used(buf)) return NULL;
	if (buf->head + n < RINGBUF_SIZE) {
		return buf->data + buf->head;
	}
	size_t left_len = RINGBUF_SIZE - buf->head;
	buf->read = malloc(n);
	memcpy(buf->read, buf->data + buf->head, left_len);
	memcpy(buf->read + left_len, buf->data, n - left_len);
	return buf->read;
}

void ringbuf_read_done(struct ringbuf *buf, size_t n) {
	if (buf->read) free(buf->read);
	buf->read = NULL;
	buf->head = (buf->head + n) & RINGBUF_MASK;
}

void *ringbuf_write(struct ringbuf *buf, size_t n) {
	if (n > ringbuf_avail(buf)) return NULL;
	if (buf->tail + n < RINGBUF_SIZE) {
		return buf->data + buf->tail;
	}
	return buf->write = malloc(n);
}

void ringbuf_write_done(struct ringbuf *buf, size_t n) {
	if (buf->write) {
		size_t left_len = RINGBUF_SIZE - buf->tail;
		memcpy(buf->data + buf->tail, buf->write, left_len);
		memcpy(buf->data, buf->write + left_len, n - left_len);
		free(buf->write);
		buf->write = NULL;
	}
	buf->tail = (buf->tail + n) & RINGBUF_MASK;
}

struct ptrqueue *ptrqueue_new(void) {
	struct ptrqueue *queue = calloc(1, sizeof(struct ptrqueue));
	queue->data = calloc(PTRQUEUE_SIZE, sizeof(void *));
	return queue;
}

void ptrqueue_free(struct ptrqueue *queue) {
	free(queue->data);
	free(queue);
}

void *ptrqueue_pop(struct ptrqueue *queue) {
	if (queue->head == queue->tail) return NULL;
	void *out = queue->data[queue->head];
	queue->head = (queue->head + 1) % PTRQUEUE_SIZE;
	return out;
}

int ptrqueue_push(struct ptrqueue *queue, void *ptr) {
	if (queue->head == queue->tail) return -1;
	queue->data[queue->tail] = ptr;
	queue->tail = (queue->tail + 1) % PTRQUEUE_SIZE;
	return 0;
}

struct arraybuf *arraybuf_new() {
	struct arraybuf *buf = calloc(1, sizeof(struct arraybuf));
	buf->cap = ARRAYBUF_INIT;
	buf->data = malloc(ARRAYBUF_INIT);
}

void arraybuf_free(struct arraybuf *buf) {
	free(buf->data);
	free(buf);
}

void *arraybuf_write(struct arraybuf *buf, size_t n) {
	if (buf->len + n >= buf->cap) {
		while (buf->len + n > buf->cap) buf->cap *= 2;
		buf->data = realloc(buf->data, buf->cap);
	}

	buf->len += n;
	return buf->data + buf->len - n;
}

void *arraybuf_peek(struct arraybuf *buf, size_t n) {
	if (buf->off + n >= buf->len) return NULL;
	return buf->data + buf->off;
}

void *arraybuf_read(struct arraybuf *buf, size_t n) {
	if (buf->off + n >= buf->len) return NULL;
	buf->off += n;
	return buf->data + buf->off - n;
}

int arraybuf_read_one(struct arraybuf *buf, uint8_t *o) {
	if (buf->off + 1 >= buf->len) return 0;
	*o = buf->data[buf->off++];
	return 1;
}

