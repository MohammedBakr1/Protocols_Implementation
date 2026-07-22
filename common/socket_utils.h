#ifndef SOCKET_UTILS_H
#define SOCKET_UTILS_H

int create_server_socket(int port, int backlog);


int connect_to_server(const char *host, int port);


char *read_until_close(int fd, size_t *out_len);


int send_all(int fd, const char *buf, size_t len);


#endif
