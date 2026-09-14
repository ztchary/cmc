CFLAGS = -Wall -Werror -std=c99 -pedantic -Isrc
CLIBS = -lpthread -lz

.PHONY: all
all: clean server

server: src/*.c src/*.h
	$(CC) -o server src/*.c $(CLIBS)

clean:
	rm -f server
