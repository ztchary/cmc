#include <errno.h>
#include <string.h>
#include <zlib.h>
#include "proto.h"
#include "packet.h"
#include "crypto.h"
#include "swrap.h"
#include "client.h"

#define NET_BUF_SIZE 4096

uint8_t inflate_buf[MAX_INFLATE_SIZE];

struct client_data *client_new(int fd) {
	struct client_data *c = calloc(1, sizeof(struct client_data));
	c->fd = fd;
	c->state = CLIENT_STATE_HANDSHAKE;
	c->pkt_in = ptrqueue_new();
	c->pkt_out = ptrqueue_new();
	c->net_in = ringbuf_new();
	c->net_out = ringbuf_new();
}

void client_free(struct client_data *c) {
	if (c->fd) close(c->fd);

	if (c->encrypt) {
		EVP_CIPHER_CTX_free(c->enc_ctx);
		EVP_CIPHER_CTX_free(c->dec_ctx);
	}

	struct arraybuf *buf;
	while (buf = ptrqueue_pop(c->pkt_in)) arraybuf_free(buf);
	while (buf = ptrqueue_pop(c->pkt_out)) arraybuf_free(buf);
	ptrqueue_free(c->pkt_in);
	ptrqueue_free(c->pkt_out);
	ringbuf_free(c->net_in);
	ringbuf_free(c->net_out);
}

int client_prepare_packets(struct client_data *c) {
}

int client_dec_net_in(struct client_data *c) {
	uint8_t net_buf[NET_BUF_SIZE];
	uint8_t *rw;
	int res;

	for (;;) {
		res = recv(c->fd, net_buf, sizeof(net_buf), 0);

		if (res <= 0) {
			if (errno == EAGAIN || errno == EWOULDBLOCK) break;
			return -1;
		}

		rw = ringbuf_write(c->net_in, res);
		
		if (c->dec_ctx) {
			EVP_DecryptUpdate(c->dec_ctx, rw, &res, net_buf, res);
		} else {
			memcpy(rw, net_buf, res);
		}

		ringbuf_write_done(c->net_in, res);
	}

	for (;;) {

	}
}

int client_enc_net_out(struct client_data *c) {
	uint8_t *rw;
	size_t send_len;
	int res;

	for (;;) {
		struct arraybuf *pkt = ptrqueue_pop(c->pkt_out);
		if (!pkt) break;
		struct arraybuf *final = arraybuf_new();
		if (!c->compress) {
			proto_enc_varint(final, pkt->len);
			proto_enc_bytes(final, pkt->data, pkt->len);
			goto encrypt_pkt;
		}

		if (pkt->len < c->threshold) {
			proto_enc_varint(final, proto_len_varint(0) + pkt->len);
			proto_enc_varint(final, 0);
			proto_enc_bytes(final, pkt->data, pkt->len);
			goto encrypt_pkt;
		}

		size_t comp_len = compressBound(pkt->len);
		uint8_t *comp = malloc(comp_len);

		res = compress(comp, &comp_len, pkt->data, pkt->len);
		if (res != Z_OK) {
			free(comp);
			arraybuf_free(pkt);
			arraybuf_free(final);
			return -1;
		}

		proto_enc_varint(final, proto_len_varint(pkt->len) + comp_len);
		proto_enc_varint(final, pkt->len);
		proto_enc_bytes(final, comp, comp_len);

	encrypt_pkt:
		int len = final->len;
		rw = ringbuf_write(c->net_out, final->len);
		if (c->encrypt) {
			EVP_EncryptUpdate(c->enc_ctx, rw, &len, final->data, len);
		} else {
			memcpy(rw, final->data, len);
		}
		ringbuf_write_done(c->net_out, len);
		arraybuf_free(pkt);
		arraybuf_free(final);
	}

	for (;;) {
		send_len = ringbuf_used(c->net_out);
		if (!send_len) break;
		send_len = send_len > NET_BUF_SIZE ? send_len : NET_BUF_SIZE;

		rw = ringbuf_read(c->net_out, send_len);
		send_len = send(c->fd, rw, send_len, 0);
		ringbuf_read_done(c->net_out, send_len);
		if (!send_len) break;
	}

	return 0;
}

