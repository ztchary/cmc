#include <pthread.h>
#include <stdio.h>
#include <sys/epoll.h>
#include <time.h>
#include "crypto.h"
#include "packet.h"
#include "swrap.h"

#define PORT 25565
#define MAX_EVENTS 64

RSA *rsa_private;
uint8_t rsa_public;
size_t rsa_public_len;

#include <stdio.h>
#include <inttypes.h>
#include <time.h>

int64_t get_time_us(void) {
	struct timespec ts;
	timespec_get(&ts, TIME_UTC);
	return ((int64_t)ts.tv_sec) * 1000000 + ((int64_t)ts.tv_nsec) / 1000;
}


int main() {
	int sock_fd = swrap_listen(PORT);
	if (sock_fd < 0) return -1;
	int epoll_fd = epoll_create1(0);

	struct epoll_event ev;
	ev.events = EPOLLIN;
	ev.data.fd = sock_fd;
	epoll_ctl(epoll_fd, EPOLL_CTL_ADD, sock_fd, &ev);

	crypto_crc32c_init();
	rsa_private = crypto_gen_keypair(rsa_public, rsa_public_len);

	struct sockaddr_in c_addr = { 0 };
	socklen_t socklen;
	int client;

	struct epoll_event events[MAX_EVENTS];

	int64_t sleep;
	while (1) {
		sleep = get_time_us();

		int n_events = epoll_wait(epoll_fd, events, MAX_EVENTS, 0);
		for (int i = 0; i < n_events; i++) {
			ev = events[i];
			if (ev.data.fd == sock_fd) {
				int c_fd = accept(sock_fd, (struct sockaddr *)&c_addr, &socklen);
				if (c_fd < 0) continue;

				swrap_nonblock(c_fd);
				struct client_data *client = client_new(c_fd);

				ev.events = EPOLLIN | EPOLLET;
				ev.data.ptr = client;

				epoll_ctl(epoll_fd, EPOLL_CTL_ADD, c_fd, &ev);
				continue;
			}

			struct client_data *client = (struct client_data *)ev.data.ptr;
			if (client_handle_data(client) != 0) {
				client_disconnect(client);
			}
		}

		sleep = 50000 - (get_time_us() - sleep);
		if (sleep > 0) usleep(sleep);
	}
}
