#include <stdint.h>

#define MAX_STAGED_SIZE   4194302

uint8_t inflate_buf[MAX_INFLATED_SIZE];

struct client_data {
	int fd;
	enum pkt_state state;
	int substate;
	int compress;
	int encrypt;
	EVP_CIPHER_CTX *enc_ctx;
	EVP_CIPHER_CTX *dec_ctx;
	int threshold;
	int username_len;
	char username[16];
	uint8_t uuid[16];
	size_t pkt_cap;
	uint8_t *pkt_buf;
	size_t stage_in_len;
	size_t stage_out_len;
	uint8_t stage_in[MAX_STAGED_SIZE];
	uint8_t stage_out[MAX_STAGED_SIZE];
};

struct client_data *client_new(int);
void client_handle_data(struct client_data *);

