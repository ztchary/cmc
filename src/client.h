#pragma once
#include <stdint.h>
#include "crypto.h"
#include "pktid.h"
#include "data.h"

#define MAX_INFLATE_SIZE 8388604

extern uint8_t inflate_buf[MAX_INFLATE_SIZE];

enum client_state {
	CLIENT_STATE_HANDSHAKE = 0,
	CLIENT_STATE_STATUS = 1,
	CLIENT_STATE_LOGIN = 2,
	CLIENT_STATE_CONFIG = 3,
	CLIENT_STATE_PLAY = 4,
};

struct client_data {
	int fd;
	enum client_state state;
	int substate;
	int compress;
	int encrypt;
	EVP_CIPHER_CTX *enc_ctx;
	EVP_CIPHER_CTX *dec_ctx;
	int threshold;
	int username_len;
	char username[16];
	uint8_t uuid[16];
	struct ptrqueue *pkt_in;
	struct ptrqueue *pkt_out;
	struct ringbuf *net_in;
	struct ringbuf *net_out;
};

struct client_data *client_new(int);
void client_free(struct client_data *);

int client_dec_net_in(struct client_data *c);
int client_enc_net_out(struct client_data *c);

