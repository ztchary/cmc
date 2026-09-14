#include <errno.h>
#include <zlib.h>
#include "proto.h"
#include "packet.h"
#include "crypto.h"
#include "client.h"

struct client_data *client_new(int fd) {
	struct client_data *c = malloc(sizeof(client_data));
	c->fd = fd;
	c->state = PKT_STATE_HANDSHAKE;
	c->in_buf = c->stage_buf;
}

int client_handle_packets(struct client_data *c) {
	size_t rem_len;
	int pack_len;
	size_t pack_read;
	int inf_len;
	int inf_len_cmp;
	size_t inf_read;
	size_t off = 0;
	int res;
	char *pos;

	while (off < c->stage_len) {
		rem_len = c->stage_len - off;
		pos = c->stage_buf + off;
		pack_read = packet_dec_varint(&pos, &rem_len, &pack_len);

		if (pack_read == 0) break;
		if (pack_len > MAX_PACKET_SIZE || pack_len < 0) return -1;

		if (pack_len > rem_len) break;

		if (!c->compress) {
			if (packet_dispatch(c, pos, pack_len) != 0) return -1;
			off += pack_len + pack_read;
			continue;
		}

		inf_read = packet_dec_varint(&pos, &pack_len, &inf_len);
		if (inf_read == 0) {
			if (packet_dispatch(c, pos, pack_len) != 0) return -1;
			off += pack_len + pack_read + inf_read;
			continue;
		}
		if (inf_len > MAX_INFLATED_SIZE || inf_len < 0) return -1;

		inf_len_cmp = inf_len;
		res = uncompress(inflate_buf, &inf_len_cmp, pos, pack_len);
		if (res != Z_OK || inf_len != inf_len_cmp) return -1;

		if (packet_dispatch(c, inflate_buf, inf_len) != 0) return -1;
		off += pack_len + pack_read + inf_read;
	}

	c->stage_len -= off;
	memmove(c->stage_buf, c->stage_buf + off, c->stage_len);
}

int client_net_in(struct client_data *c) {
	uint8_t net_buf[4096];
	size_t res;

	while (1) {
		res = recv(c->fd, net_buf, sizeof(net_buf), 0);

		if (res <= 0) {
			if (errno == EAGAIN || errno = EWOULDBLOCK) break;
			return -1;
		}

		if (c->stage_len + res < MAX_STAGED_SIZE) return -1;
		
		if (c->encrypt) {
			EVP_DecryptUpdate(c->dec_ctx,
					c->stage_buf + c->stage_len, &res,
					net_buf, res);
		} else {
			memcpy(c->stage_buf + c->stage_len, net_buf, res);
		}

		c->stage_len += res;
	}

	return client_handle_packets(c);
}

int client_net_out(struct client_data *c, size_t len) {
	uint8_t *pkt = NULL;
	size_t pkt_cap = 0;
	size_t pkt_len = 0;
	uint8_t *sbuf = pkt;
	int res;

	if (!c->compress) {
		pkt_len += proto_enc_varint(&pkt, &pkt_cap, pkt_len, len);
		pkt_len += proto_enc_bytes(&pkt, &pkt_cap, pkt_len, c->packet_buf, len);
		goto encrypt_pkt;
	}

	if (len < c->threshold) {
		pkt_len += proto_enc_varint(&pkt, &pkt_cap, pkt_len, proto_varint_len(0) + len);
		pkt_len += proto_enc_varint(&pkt, &pkt_cap, pkt_len, 0);
		pkt_len += proto_enc_bytes(&pkt, &pkt_cap, pkt_len, c->packet_buf, len);
		goto encrypt_pkt;
	}

	size_t comp_len = compressBound(len);
	uint8_t *comp = malloc(comp_len);

	res = compress(comp, &comp_len, pkt, pkt_len);
	if (res != Z_OK) {
		free(comp);
		return -1;
	}

	pkt_len += proto_enc_varint(&pkt, &pkt_cap, pkt_len, proto_len_varint(len) + comp_len);
	pkt_len += proto_enc_varint(&pkt, &pkt_cap, pkt_len, len);
	pkt_len += proto_enc_bytes(&pkt, &pkt_cap, pkt_len, comp, comp_len);

encrypt_pkt:
	if (c->encrypt) {
		sbuf = malloc(pkt_len);
		EVP_EncryptUpdate(c->enc_ctx, sbuf, &pkt_len, pkt, pkt_len);
	}

	res = send(c->fd, sbuf, pkt_len, 0);
	if (c->encrypt) free(sbuf);
	free(pkt);
	return 0;

}

