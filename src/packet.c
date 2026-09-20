#include <stdio.h>
#include <string.h>
#include <zlib.h>
#include "crypto.h"
#include "proto.h"
#include "pktid.h"
#include "packet.h"
#include "client.h"

int packet_state_handshake(int op, struct client_data *c, struct arraybuf *buf) {
	int version;
	int intent;
	if (op != CS_HS_INTENTION) return -1;
	if (proto_dec_varint(buf, &version) == 0) return -1;
	if (version != PROTOCOL_VERS) return -1;
	proto_dec_string(buf, NULL, NULL);
	proto_dec_ushort(buf, NULL);
	proto_dec_varint(buf, &intent);
	if (intent > 3 || intent < 1) return -1;
	c->state = intent == 1 ? CLIENT_STATE_STATUS : CLIENT_STATE_LOGIN;
	c->substate = 0;
	return 0;
}

int packet_state_status(int op, struct client_data *c, struct arraybuf *buf) { 
	struct arraybuf *obuf = arraybuf_new();
	if (op == CS_STATUS_PING) {
		uint64_t ts;
		proto_enc_varint(obuf, SC_STATUS_PONG);
		if (proto_dec_long(buf, &ts) != sizeof(ts)) {
			arraybuf_free(obuf);
			return -1;
		}
		proto_enc_long(obuf, ts);
		printf("%lld\n", ts);
	} else if (op == CS_STATUS_REQ) {
		const char *resp = "{\"version\":{\"name\":\"26.2\",\"protocol\":776},\"description\":\"woah\"}";
		proto_enc_varint(obuf, SC_STATUS_RES);
		proto_enc_string(obuf, resp, strlen(resp));
	} else {
		arraybuf_free(obuf);
		return -1;
	}
	ptrqueue_push(c->pkt_out, obuf);
	return 0;
}

int packet_login_hello(struct client_data *c, struct arraybuf *buf) {
	size_t vb_len;
	if (c->substate++ != 0) return -1;

	uint8_t *username;
	size_t username_len;

	if (proto_dec_string(buf, &username, &username_len) == 0) return -1;
	if (proto_dec_uuid(buf, c->uuid) == 0) return -1;

	if (username_len > 16 || username_len < 1) return -1;
	memcpy(c->username, username, username_len);
	c->username_len = username_len;
	return 0;
}

int packet_login_key(struct client_data *c, struct arraybuf *buf) { return -1; };
int packet_login_custom_query_answer(struct client_data *c, struct arraybuf *buf) { return -1; };
int packet_login_acknowledged(struct client_data *c, struct arraybuf *buf) { return -1; };
int packet_login_cookie_response(struct client_data *c, struct arraybuf *buf) { return -1; };

int packet_state_login(int op, struct client_data *c, struct arraybuf *buf) {
	switch (op) {
		case CS_LOGIN_HELLO:               return packet_login_hello(c, buf);
		case CS_LOGIN_KEY:                 return packet_login_key(c, buf);
		case CS_LOGIN_CUSTOM_QUERY_ANSWER: return packet_login_custom_query_answer(c, buf);
		case CS_LOGIN_LOGIN_ACKNOWLEDGED:  return packet_login_acknowledged(c, buf);
		case CS_LOGIN_COOKIE_RESPONSE:     return packet_login_cookie_response(c, buf);
		default: return -1;
	}
}

int packet_state_config(int op, struct client_data *c, struct arraybuf *buf) { return -1; }

int packet_state_play(int op, struct client_data *c, struct arraybuf *buf) { return -1; }

int packet_dispatch(struct client_data *c, struct arraybuf *buf) {
	int op;
	if (proto_dec_varint(buf, &op) == 0) return -1;
	switch(c->state) {
		case CLIENT_STATE_HANDSHAKE: return packet_state_handshake(op, c, buf);
		case CLIENT_STATE_STATUS:    return packet_state_status(op, c, buf);
		case CLIENT_STATE_LOGIN:     return packet_state_login(op, c, buf);
		case CLIENT_STATE_CONFIG:    return packet_state_config(op, c, buf);
		case CLIENT_STATE_PLAY:      return packet_state_play(op, c, buf);
		default: return -1;
	}
}

/*
void proto_enc_wire(struct client_data *c, struct arraybuf *buf) {
	size_t buflen = buf->pos - buf->data;
	struct pktbuf out = packet_new();
	if (!c->compress) {
		proto_enc_varint(&out, buflen);
		proto_enc_bytes(&out, buf->data, buflen);
		goto send_pkt;
	}

	if (buflen < c->threshold) {
		proto_enc_varint(&out, varint_len(0) + buflen);
		proto_enc_varint(&out, 0);
		proto_enc_bytes(&out, buf->data, buflen);
		goto send_pkt;
	}

	size_t comp_len = compressBound(buflen);
	uint8_t *comp = malloc(comp_len);

	int res = compress(comp, &comp_len, buf->data, buflen);
	if (res != Z_OK) {
		free(comp);
		return;
	}

	proto_enc_varint(&out, varint_len(buflen) + comp_len);
	proto_enc_varint(&out, buflen);
	proto_enc_bytes(&out, comp, comp_len);
	free(comp);

send_pkt:
	swrap_send(c->fd, out.data, out.data - out.pos, client->enc_ctx);
	packet_free(&out);
}
*/

