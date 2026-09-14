#pragma once
#include <stddef.h>
#include "crypto.h"

int swrap_listen(int);
void swrap_nonblock(int);
int swrap_send(int, char *, int, EVP_CIPHER_CTX *);
int swrap_recv(int, char *, size_t, EVP_CIPHER_CTX *);

