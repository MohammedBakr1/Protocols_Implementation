#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "../common/socket_utils.h"

#define PORT 7070

static const char *root_dir = "gopher_root";

static void send_menu(int client_fd) {
    char response[8192] = {0};
    size_t offset = 0;

    offset += snprintf(response + offset, sizeof(response) - offset,
        "1Welcome Menu\tREADME\tlocalhost\t%d\r\n", PORT);
    offset += snprintf(response + offset, sizeof(response) - offset,
        "0About this server\tabout.txt\tlocalhost\t%d\r\n", PORT);
    offset += snprintf(response + offset, sizeof(response) - offset,
        ".\r\n");

    send_all(client_fd, response, offset);
}

static void send_file(int client_fd, const char *selector) {
    char path[512];
    snprintf(path, sizeof(path), "%s/%s", root_dir, selector);

    FILE *f = fopen(path, "r");
    if (!f) {
        const char *msg = "3File not found\r\n.\r\n";
        send_all(client_fd, msg, strlen(msg));
        return;
    }

    char buf[4096];
    size_t n;
    while ((n = fread(buf, 1, sizeof(buf), f)) > 0) {
        send_all(client_fd, buf, n);
    }
    fclose(f);

    send_all(client_fd, ".\r\n", 3);
}

static void handle_client(int client_fd) {
    char selector[512] = {0};
    ssize_t n = recv(client_fd, selector, sizeof(selector) - 1, 0);
    if (n <= 0) {
        close(client_fd);
        return;
    }
    selector[n] = '\0';

    char *cr = strpbrk(selector, "\r\n");
    if (cr) *cr = '\0';

    printf("[gopher] selector requested: \"%s\"\n", selector);

    if (strlen(selector) == 0) {
        send_menu(client_fd);
    } else {
        send_file(client_fd, selector);
    }

    close(client_fd);
}

int main(int argc, char *argv[]) {
    if (argc > 1) {
        root_dir = argv[1];
    }

    setvbuf(stdout, NULL, _IONBF, 0);

    int server_fd = create_server_socket(PORT, 10);
    if (server_fd < 0) {
        fprintf(stderr, "Failed to start server\n");
        return 1;
    }

    printf("Gopher server running on port %d (root: %s)\n", PORT, root_dir);
    printf("Try: printf '\\r\\n' | nc localhost %d\n", PORT);

    while (1) {
        struct sockaddr_in client_addr;
        socklen_t addr_len = sizeof(client_addr);
        int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &addr_len);
        if (client_fd < 0) {
            perror("accept");
            continue;
        }

        printf("[gopher] New connection from %s\n", inet_ntoa(client_addr.sin_addr));
        handle_client(client_fd);
    }

    close(server_fd);
    return 0;
}
