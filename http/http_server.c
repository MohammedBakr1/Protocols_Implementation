#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include "../common/socket_utils.h"

#define PORT 8080
#define MAX_REQUEST 8192

static const char *root_dir = "www";

static const char *guess_content_type(const char *path) {
    const char *ext = strrchr(path, '.');
    if (!ext) return "application/octet-stream";

    if (strcmp(ext, ".html") == 0 || strcmp(ext, ".htm") == 0) return "text/html";
    if (strcmp(ext, ".txt") == 0) return "text/plain";
    if (strcmp(ext, ".css") == 0) return "text/css";
    if (strcmp(ext, ".js") == 0) return "application/javascript";
    if (strcmp(ext, ".json") == 0) return "application/json";
    if (strcmp(ext, ".png") == 0) return "image/png";
    if (strcmp(ext, ".jpg") == 0 || strcmp(ext, ".jpeg") == 0) return "image/jpeg";

    return "application/octet-stream";
}

static int parse_request_line(const char *request, char *method, char *path) {
    const char *line_end = strstr(request, "\r\n");
    if (!line_end) return -1;

    int matched = sscanf(request, "%15s %255s", method, path);
    if (matched != 2) return -1;

    return 0;
}

static void send_response(int client_fd, int status_code, const char *status_text,
                           const char *content_type, const char *body, size_t body_len) {
    char header[512];
    int header_len = snprintf(header, sizeof(header),
        "HTTP/1.1 %d %s\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %zu\r\n"
        "Connection: close\r\n"
        "\r\n",
        status_code, status_text, content_type, body_len);

    send_all(client_fd, header, (size_t)header_len);
    if (body_len > 0) {
        send_all(client_fd, body, body_len);
    }
}

static void send_404(int client_fd) {
    const char *body = "<html><body><h1>404 Not Found</h1></body></html>";
    send_response(client_fd, 404, "Not Found", "text/html", body, strlen(body));
}

static void send_400(int client_fd) {
    const char *body = "<html><body><h1>400 Bad Request</h1></body></html>";
    send_response(client_fd, 400, "Bad Request", "text/html", body, strlen(body));
}

static void send_405(int client_fd) {
    const char *body = "<html><body><h1>405 Method Not Allowed</h1></body></html>";
    send_response(client_fd, 405, "Method Not Allowed", "text/html", body, strlen(body));
}

static void serve_file(int client_fd, const char *path) {
    char full_path[600];

    if (strcmp(path, "/") == 0) {
        snprintf(full_path, sizeof(full_path), "%s/index.html", root_dir);
    } else {
        snprintf(full_path, sizeof(full_path), "%s%s", root_dir, path);
    }

    if (strstr(path, "..") != NULL) {
        send_400(client_fd);
        return;
    }

    FILE *f = fopen(full_path, "rb");
    if (!f) {
        send_404(client_fd);
        return;
    }

    struct stat st;
    if (stat(full_path, &st) != 0) {
        fclose(f);
        send_404(client_fd);
        return;
    }

    char *body = malloc((size_t)st.st_size);
    if (!body) {
        fclose(f);
        send_response(client_fd, 500, "Internal Server Error", "text/plain", "", 0);
        return;
    }

    size_t read_bytes = fread(body, 1, (size_t)st.st_size, f);
    fclose(f);

    send_response(client_fd, 200, "OK", guess_content_type(full_path), body, read_bytes);
    free(body);
}

static void handle_client(int client_fd) {
    char request[MAX_REQUEST] = {0};
    ssize_t n = recv(client_fd, request, sizeof(request) - 1, 0);
    if (n <= 0) {
        close(client_fd);
        return;
    }
    request[n] = '\0';

    char method[16] = {0};
    char path[256] = {0};

    if (parse_request_line(request, method, path) != 0) {
        send_400(client_fd);
        close(client_fd);
        return;
    }

    printf("[http] %s %s\n", method, path);

    if (strcmp(method, "GET") != 0) {
        send_405(client_fd);
        close(client_fd);
        return;
    }

    serve_file(client_fd, path);
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

    printf("HTTP server running on port %d (root: %s)\n", PORT, root_dir);
    printf("Try: curl http://localhost:%d/\n", PORT);

    while (1) {
        struct sockaddr_in client_addr;
        socklen_t addr_len = sizeof(client_addr);
        int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &addr_len);
        if (client_fd < 0) {
            perror("accept");
            continue;
        }

        printf("[http] New connection from %s\n", inet_ntoa(client_addr.sin_addr));
        handle_client(client_fd);
    }

    close(server_fd);
    return 0;
}
