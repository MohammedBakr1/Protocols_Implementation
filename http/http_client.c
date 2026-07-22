#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include "../common/socket_utils.h"

static char *find_headers_end(char *buf, size_t len) {
    for (size_t i = 0; i + 3 < len; i++) {
        if (buf[i] == '\r' && buf[i+1] == '\n' && buf[i+2] == '\r' && buf[i+3] == '\n') {
            return buf + i;
        }
    }
    return NULL;
}

static long parse_content_length(const char *headers) {
    const char *p = headers;
    size_t target_len = strlen("content-length:");

    while (*p) {
        size_t i = 0;
        while (i < target_len && p[i] &&
               tolower((unsigned char)p[i]) == "content-length:"[i]) {
            i++;
        }
        if (i == target_len) {
            return atol(p + target_len);
        }
        p++;
    }
    return -1;
}

int main(int argc, char *argv[]) {
    if (argc < 4) {
        fprintf(stderr, "Usage: %s <host> <port> <path>\n", argv[0]);
        fprintf(stderr, "Example: %s localhost 8080 /\n", argv[0]);
        return 1;
    }

    const char *host = argv[1];
    int port = atoi(argv[2]);
    const char *path = argv[3];

    int fd = connect_to_server(host, port);
    if (fd < 0) {
        fprintf(stderr, "Failed to connect to %s:%d\n", host, port);
        return 1;
    }

    char request[1024];
    int req_len = snprintf(request, sizeof(request),
        "GET %s HTTP/1.1\r\n"
        "Host: %s\r\n"
        "Connection: close\r\n"
        "\r\n",
        path, host);

    if (send_all(fd, request, (size_t)req_len) != 0) {
        fprintf(stderr, "Failed to send request\n");
        close(fd);
        return 1;
    }

    size_t total_len = 0;
    char *response = read_until_close(fd, &total_len);
    close(fd);

    if (!response) {
        fprintf(stderr, "Failed to read response\n");
        return 1;
    }

    char *headers_end = find_headers_end(response, total_len);
    if (!headers_end) {
        fprintf(stderr, "Invalid response: no clear end of headers\n");
        free(response);
        return 1;
    }

    size_t headers_len = (size_t)(headers_end - response);
    char *body_start = headers_end + 4;
    size_t body_len_from_socket = total_len - headers_len - 4;

    printf("--- Headers ---\n");
    fwrite(response, 1, headers_len, stdout);
    printf("\n\n");

    long declared_len = parse_content_length(response);
    if (declared_len >= 0) {
        printf("--- Declared Content-Length: %ld, actually received: %zu ---\n",
               declared_len, body_len_from_socket);
    }

    printf("--- Body ---\n");
    fwrite(body_start, 1, body_len_from_socket, stdout);
    printf("\n");

    free(response);
    return 0;
}
