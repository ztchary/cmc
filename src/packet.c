#include <stdio.h>
#include <zlib.h>
#include "crypto.h"
#include "proto.h"
#include "pktid.h"
#include "swrap.h"
#include "packet.h"

int packet_state_handshake(int op, struct client_data *c, uint8_t *buf, size_t n) {
	int version;
	int intent;
	if (op != CS_HS_INTENTION) return -1;
	if (packet_dec_varint(&buf, &n, &version) < 1) return -1;
	if (version != PROTOCOL_VERS) return -1;
	packet_dec_string(&buf, &n, NULL, NULL);
	packet_dec_ushort(&buf, &n, NULL);
	packet_dec_varint(&buf, &n, &intent);
	if (intent > 3 || intent < 1) return -1;
	c->state = intent == 1 ? PKT_STATE_STATUS : PKT_STATE_LOGIN;
	c->substate = 0;
	return 0;
}

int packet_state_status(int op, struct client_data *c, uint8_t *buf, size_t n) { 
	uint8_t pkt;
	size_t pkt_len;
	size_t ;
	if (op == CS_STATUS_PING) {
		pktpacket_enc_varint(&ret, SC_STATUS_PONG);
		uint64_t ts = packet_dec_long(buf);
		printf("%lld\n", ts);
		packet_enc_long(&ret, ts);
	} else if (op == CS_STATUS_REQ) {
		const char *resp = "{\"version\":{\"name\":\"26.2\",\"protocol\":776},\"description\":\"woah\"}";
		packet_enc_varint(&ret, SC_STATUS_RES);
		packet_enc_varbytes(&ret, resp, strlen(resp));
	} else return -1;
	packet_enc_wire(c->fd, &ret, client->compress, client->threshold);
	packet_free(&ret);
	return 0;
}

int packet_login_hello(struct client_data *c, uint8_t *buf, size_t n) {
	size_t vb_len;
	if (c->substate != 0) return -1;
	c->substate++;

	const char *username = packet_dec_varbytes(buf, &vb_len);
	const UUID uuid = packet_dec_uuid(buf);

	if (vb_len > 16) return -1;
	memcpy(c->username, username, vb_len);
	c->username_len = vb_len;

	memcpy(c->uuid, uuid, 16);
}

int packet_login_key(struct client_data *c, uint8_t *buf, size_t n) { return -1; };
int packet_login_custom_query_answer(struct client_data *c, uint8_t *buf, size_t n) { return -1; };
int packet_login_acknowledged(struct client_data *c, uint8_t *buf, size_t n) { return -1; };
int packet_login_cookie_response(struct client_data *c, uint8_t *buf, size_t n) { return -1; };

int packet_state_login(int op, struct client_data *c, uint8_t *buf, size_t n) {
	switch (op) {
		case CS_LOGIN_HELLO:               return packet_login_hello(c, buf, n);
		case CS_LOGIN_KEY:                 return packet_login_key(c, buf, n);
		case CS_LOGIN_CUSTOM_QUERY_ANSWER: return packet_login_custom_query_answer(c, buf, n);
		case CS_LOGIN_LOGIN_ACKNOWLEDGED:  return packet_login_acknowledged(c, buf, n);
		case CS_LOGIN_COOKIE_RESPONSE:     return packet_login_cookie_response(c, buf, n);
	}
}

int packet_state_config(int op, struct client_data *c, uint8_t *buf, size_t n) { return -1; }

int packet_state_play(int op, struct client_data *c, uint8_t *buf, size_t n) { return -1; }

int packet_dispatch(struct client_data *c, uint8_t *buf, size_t n) {
	int op;
	if (proto_dec_varint(&buf, &n, &op) < 1) return -1;
	switch(c->state) {
		case PKT_STATE_HANDSHAKE: return packet_state_handshake(op, c, buf, n);
		case PKT_STATE_STATUS:    return packet_state_status(op, c, buf, n);
		case PKT_STATE_LOGIN:     return packet_state_login(op, c, buf, n);
		case PKT_STATE_CONFIG:    return packet_state_config(op, c, buf, n);
		case PKT_STATE_PLAY:      return packet_state_play(op, c, buf, n);
		default: return -1;
	}
}

/*
void packet_enc_wire(struct client_data *c, uint8_t *buf, size_t n) {
	size_t buflen = buf->pos - buf->data;
	struct pktbuf out = packet_new();
	if (!c->compress) {
		packet_enc_varint(&out, buflen);
		packet_enc_bytes(&out, buf->data, buflen);
		goto send_pkt;
	}

	if (buflen < c->threshold) {
		packet_enc_varint(&out, varint_len(0) + buflen);
		packet_enc_varint(&out, 0);
		packet_enc_bytes(&out, buf->data, buflen);
		goto send_pkt;
	}

	size_t comp_len = compressBound(buflen);
	uint8_t *comp = malloc(comp_len);

	int res = compress(comp, &comp_len, buf->data, buflen);
	if (res != Z_OK) {
		free(comp);
		return;
	}

	packet_enc_varint(&out, varint_len(buflen) + comp_len);
	packet_enc_varint(&out, buflen);
	packet_enc_bytes(&out, comp, comp_len);
	free(comp);

send_pkt:
	swrap_send(c->fd, out.data, out.data - out.pos, client->enc_ctx);
	packet_free(&out);
}
*/

