#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <fcntl.h>
#include "swrap.h"

void swrap_nonblock(int fd) {
	int flags = fcntl(fd, F_GETFL, 0);
	fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

int swrap_listen(int port) {
	int fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd < 0) return -1;
	struct sockaddr_in addr = { 0 };
	socklen_t socklen;

	int opt = 1;
	setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	swrap_nonblock(fd);

	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = INADDR_ANY;

	if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		close(fd);
		return -1;
	}

	if (listen(fd, 5) < 0) {
		close(fd);
		return -1;
	}

	return fd;
}

int swrap_send(int fd, char *src, int n, EVP_CIPHER_CTX *enc_ctx) {
	char *sbuf = src;
	int res;

	if (enc_ctx) {
		sbuf = malloc(n);
		EVP_EncryptUpdate(enc_ctx, sbuf, &n, src, n);
	}

	res = send(fd, sbuf, n, 0);
	if (enc_ctx) free(sbuf);
	return res;
}
	
int swrap_recv(int fd, char *dest, size_t n, EVP_CIPHER_CTX *dec_ctx) {
	int total = 0;
	char *rbuf;
	int res;

	rbuf = dec_ctx ? malloc(n) : dest;

	while (total < n) {
		res = recv(fd, rbuf + total, n - total, 0);
		if (res < 1) return 0;
		total += res;
	}

	if (dec_ctx) {
		EVP_DecryptUpdate(dec_ctx, dest, &res, rbuf, res);
		free(rbuf);
	}

	return total;
}
