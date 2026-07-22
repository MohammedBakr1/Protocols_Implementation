#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../common/socket_utils.h"

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <host> <port> [selector]\n", argv[0]);
        fprintf(stderr, "Example: %s localhost 7070 about.txt\n", argv[0]);
        return 1;
    }

    const char *host = argv[1];
    int port = atoi(argv[2]);
    const char *selector = (argc >= 4) ? argv[3] : "";

    int fd = connect_to_server(host, port);
    if (fd < 0) {
        fprintf(stderr, "Failed to connect to %s:%d\n", host, port);
        return 1;
    }

    char request[600];
    int req_len = snprintf(request, sizeof(request), "%s\r\n", selector);

    if (send_all(fd, request, (size_t)req_len) != 0) {
        fprintf(stderr, "Failed to send request\n");
        close(fd);
        return 1;
    }

    size_t response_len = 0;
    char *response = read_until_close(fd, &response_len);
    close(fd);

    if (!response) {
        fprintf(stderr, "Failed to read response\n");
        return 1;
    }

    printf("%s", response);
    free(response);

    return 0;
}
