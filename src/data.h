#include <stdint.h>
#include <stddef.h>

#define RINGBUF_SIZE (1<<22)
#define RINGBUF_MASK ((1<<22)-1)

struct ringbuf {
	volatile size_t head;
	volatile size_t tail;
	uint8_t *read;
	uint8_t *write;
	uint8_t *data;
};

struct ringbuf *ringbuf_new(void);
void ringbuf_free(struct ringbuf *);

size_t ringbuf_used(struct ringbuf *);

void *ringbuf_read(struct ringbuf *, size_t);
void ringbuf_read_done(struct ringbuf *, size_t);
void *ringbuf_write(struct ringbuf *, size_t);
void ringbuf_write_done(struct ringbuf *, size_t);

#define PTRQUEUE_SIZE 32

struct ptrqueue {
	volatile size_t head;
	volatile size_t tail;
	void **data;
};

struct ptrqueue *ptrqueue_new(void);
void ptrqueue_free(struct ptrqueue *);

void *ptrqueue_pop(struct ptrqueue *);
int ptrqueue_push(struct ptrqueue *, void *);

#define ARRAYBUF_INIT 256

struct arraybuf {
	size_t cap;
	size_t len;
	size_t off;
	uint8_t *data;
};

struct arraybuf *arraybuf_new(void);
void arraybuf_free(struct arraybuf *);

void *arraybuf_write(struct arraybuf *, size_t);
void *arraybuf_peek(struct arraybuf *, size_t);
void *arraybuf_read(struct arraybuf *, size_t);
int arraybuf_read_one(struct arraybuf *, uint8_t *);

