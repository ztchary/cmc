#pragma once
#include <stdint.h>
#include "client.h"
#include "data.h"

#define PROTOCOL_VERS 776
#define MAX_PKT_LEN 2097151

int packet_dispatch(struct client_data *, struct arraybuf *);

