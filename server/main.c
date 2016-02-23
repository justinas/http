#ifdef __APPLE__
#include <netinet/in.h> // OS X
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

void fatal(const char *msg) {
    fputs(msg, stderr);
    exit(1);
}

void efatal(const char *prepend) {
    perror(prepend);
    exit(1);
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fatal("Usage: ./server port");
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[1]));

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        efatal("socket");
    }
    int one = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one)) < 0) {
        efatal("setsockopt");
    }
    if (bind(server_fd, (struct sockaddr*) &server_addr, sizeof(struct sockaddr)) < 0) {
        efatal("bind");
    }
    if (listen(server_fd, 5) < 0) {
        efatal("listen");
    }

    struct sockaddr client_addr;
    socklen_t client_addr_len;
    for (;;) {
        int client_fd = accept(server_fd, &client_addr, &client_addr_len);
        if (client_fd < 0) {
            efatal("accept");
        }
        char *msg = "HTTP/1.0 200 OK\r\n\r\n";
        write(client_fd, msg, strlen(msg));
        close(client_fd);
    }
    return 0;
}
