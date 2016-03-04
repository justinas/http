#ifdef __APPLE__
#include <netinet/in.h> // OS X
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "../parser/parser.h"

void fatal(const char *msg) {
    fputs(msg, stderr);
    exit(1);
}

void efatal(const char *prepend) {
    perror(prepend);
    exit(1);
}

void handle_client(int fd) {
    parser* p = parser_new();
    for (;;) {
        char buf[1024];
        ssize_t n = recv(fd, buf, 1024, 0);
        if (n == -1) {
            efatal("recv");
        } else if (n == 0) {
            break;
        }
        int parser_status = parser_feed(p, buf, n);
        if (parser_status == -1) {
            fatal("parser error");
        }
        else if (parser_status == 1) {
            break;
        }
    }

    http_request *req = parser_request(p);
    char *msg = "HTTP/1.0 200 OK\r\n\r\n";
    char *init_html = "<!doctype html><title>Server</title>\n"
        "<table>";
    if (send(fd, msg, strlen(msg), 0) == -1) {
        efatal("send");
    }
    if (send(fd, init_html, strlen(init_html), 0) == -1) {
        efatal("send");
    }
    for (http_header *h = req->header; h != NULL; h = h->next) {
        char *init = "<tr><td>";
        if (send(fd, init, strlen(init), 0) == -1) {
            efatal("send");
        }
        if (send(fd, h->name, strlen(h->name), 0) == -1) {
            efatal("send");
        }
        char *mid = "</td><td>";
        if (send(fd, mid, strlen(mid), 0) == -1) {
            efatal("send");
        }
        if (send(fd, h->value, strlen(h->value), 0) == -1) {
            efatal("send");
        }
        char *end = "</td><td>";
        if (send(fd, end, strlen(end), 0) == -1) {
            efatal("</td></tr>");
        }
    }
    char *end_html = "</table>";
    if (send(fd, end_html, strlen(end_html), 0) == -1) {
        efatal("</td></tr>");
    }

    parser_destroy(p);
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
        handle_client(client_fd);
        close(client_fd);
    }
    return 0;
}
