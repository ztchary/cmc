#pragma once
#include <stdint.h>

#define PROTOCOL_VERS 776
#define MAX_PKT_LEN 2097151

enum pkt_state {
	PKT_STATE_HANDSHAKE = 0,
	PKT_STATE_STATUS = 1,
	PKT_STATE_LOGIN = 2,
	PKT_STATE_CONFIG = 3,
	PKT_STATE_PLAY = 4,
};

int packet_dispatch(struct client_data *, uint8_t *, size_t);

void pkt_enc_wire(int, struct pktbuf *, int, int);
int read_varint_wire(int, int32_t *);
int pkt_dec_wire(int, struct pktbuf *, int);

