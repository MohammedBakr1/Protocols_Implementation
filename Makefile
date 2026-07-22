CC = gcc
CFLAGS = -Wall -Wextra -g -std=c11

COMMON = common/socket_utils.c

.PHONY: all clean run-gopher-server run-http-server

all: gopher_server gopher_client http_server http_client

gopher_server: gopher/gopher_server.c $(COMMON)
	$(CC) $(CFLAGS) -o gopher_server gopher/gopher_server.c $(COMMON)

gopher_client: gopher/gopher_client.c $(COMMON)
	$(CC) $(CFLAGS) -o gopher_client gopher/gopher_client.c $(COMMON)

http_server: http/http_server.c $(COMMON)
	$(CC) $(CFLAGS) -o http_server http/http_server.c $(COMMON)

http_client: http/http_client.c $(COMMON)
	$(CC) $(CFLAGS) -o http_client http/http_client.c $(COMMON)

run-gopher-server: gopher_server
	./gopher_server

run-http-server: http_server
	./http_server

clean:
	rm -f gopher_server gopher_client http_server http_client
