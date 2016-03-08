#ifdef __APPLE__
#include <netinet/in.h> // OS X
#endif

#include <arpa/inet.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "../parser/parser.h"

#include "bytebuf.h"
#include "client.h"
#include "handler.h"

#define MAX_CLIENTS 20

void fatal(const char *msg) {
    fputs(msg, stderr);
    exit(1);
}

void efatal(const char *prepend) {
    perror(prepend);
    exit(1);
}

int make_server_socket(uint16_t port) {
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    inet_aton("0.0.0.0", &server_addr.sin_addr);

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        efatal("socket");
    }
    int one = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one)) < 0) {
        efatal("setsockopt");
    }
    if (fcntl(fd, F_SETFL, O_NONBLOCK, 1) == -1) {
        efatal("fcntl");
    }
    if (bind(fd, (struct sockaddr*) &server_addr, sizeof(struct sockaddr)) < 0) {
        efatal("bind");
    }
    if (listen(fd, 5) < 0) {
        efatal("listen");
    }

    return fd;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fatal("Usage: ./server port");
    }
    int server_fd = make_server_socket(atoi(argv[1]));
    int max_fd = server_fd;

    client clients[MAX_CLIENTS];
    size_t clients_len = 0;
    memset(clients, 0, MAX_CLIENTS * sizeof(client));

    for (;;) {
        fd_set readfds;
        fd_set writefds;
        FD_ZERO(&readfds);
        FD_ZERO(&writefds);

        if (clients_len == MAX_CLIENTS) {
            fputs("MAX_CLIENTS reached, not accepting another one"
                    " until someone disconnects.\n", stderr);
        } else {
            FD_SET(server_fd, &readfds);
        }

        for (size_t i = 0; i < clients_len; i++) {
            if (clients[i].parser_status == 1) {
                FD_SET(clients[i].fd, &writefds);
            } else {
                FD_SET(clients[i].fd, &readfds);
            }
        }

        if (select(max_fd+1, &readfds, &writefds, NULL, NULL) == -1) {
            efatal("select");
        }

        if (FD_ISSET(server_fd, &readfds)) {
            struct sockaddr client_addr;
            socklen_t client_addr_len;
            int client_fd = accept(server_fd, &client_addr, &client_addr_len);
            if (client_fd < 0) {
                efatal("accept");
            }
            if (client_fd > max_fd) {
                max_fd = client_fd;
            }
            clients[clients_len].fd = client_fd;
            clients[clients_len].parser = parser_new();
            clients[clients_len].parser_status = 0;
            clients_len++;
        }

        for (size_t i = 0; i < clients_len; i++) {
            int fd = clients[i].fd;

            // Reading the request
            if (FD_ISSET(fd, &readfds) && clients[i].parser_status != 1) {
                char buf[256];
                ssize_t n = recv(fd, buf, 256, 0);
                if (n == -1) {
                    efatal("recv");
                }
                if (n == 0) {
                    parser_destroy(clients[i].parser);
                    memset(&clients[i], 0, sizeof(client));
                    for (size_t j = i; j < clients_len-1; j++) {
                        clients[j] = clients[j+1];
                    }
                    clients_len--;
                    continue;
                }

                int status = parser_feed(clients[i].parser, buf, n);
                clients[i].parser_status = status;
                if (status == 1) {
                    clients[i].response = handler(&clients[i]);
                    parser_destroy(clients[i].parser);
                    clients[i].parser = NULL;
                } else if (status == -1) {
                    parser_destroy(clients[i].parser);
                    memset(&clients[i], 0, sizeof(client));
                    for (size_t j = i; j < clients_len-1; j++) {
                        clients[j] = clients[j+1];
                    }
                    clients_len--;
                    continue;
                }
            }

            // Writing the request
            if (FD_ISSET(fd, &writefds)) {
                client *c = &clients[i];
                if (c->resp_bytes_written == c->response->len) {
                    bytebuf_destroy(c->response);
                    memset(c, 0, sizeof(client));
                    for (size_t j = i; j < clients_len-1; j++) {
                        clients[j] = clients[j+1];
                    }
                    clients_len--;
                    close(fd);
                    continue;
                }

                char *buf_start = c->response->data + c->resp_bytes_written;
                size_t buf_remaining = c->response->len -= c->resp_bytes_written;
                ssize_t n = send(fd, buf_start, buf_remaining, 0);
                if (n == -1) {
                    efatal("send");
                }
                c->resp_bytes_written += n;
            }
        }
        printf("%lu clients connected.\n", clients_len);

    }
    return 0;
}
