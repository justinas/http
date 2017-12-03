#define __USE_XOPEN2K // idk
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>

#define MIN(a, b) (a) < (b) ? (a) : (b)

void fatal(const char *msg) {
    fputs(msg, stderr);
    exit(1);
}

void efatal(const char *prepend) {
    perror(prepend);
    exit(1);
}

ssize_t sendall(int fd, const char *buf, size_t n) {
    const char *buf_end = buf+n;
    while (buf < buf_end) {
        ssize_t sent = send(fd, buf, MIN(n, 4096), 0);
        if (sent < 0) {
            return sent;
        }
        buf += sent;
    }
    return n - (buf_end - buf);
}

void request(int sock, char* path) {
    char reqbuf[2048];
    char *req = "GET %s HTTP/1.0\r\n"
        "User-Agent: Client 0.1\r\n"
        "\r\n\r\n";
    if (snprintf(reqbuf, sizeof reqbuf, req, path) < 1) {
        efatal("snprintf");
    }
    puts(reqbuf);
    if (sendall(sock, reqbuf, strlen(req)) < 0) {
        efatal("sendall");
    } 

    for (;;) {
        char buf[128];
        ssize_t n = recv(sock, buf, 128, 0);
        switch (n) {
            case -1:
                efatal("recv");
            case 0:
                return;
            default:
                fwrite(buf, n, 1, stdout);
        }
    }
}


int main(int argc, char **argv) {
    if (argc != 2) {
        fatal("Usage: ./client URL");
    }

    if (strlen(argv[1]) < 8 || strncmp(argv[1], "http://", 7) != 0) {
        fatal("Invalid URL supplied.");
    }

    char host[256];
    char *host_end = NULL;
    {
        char *host_start = argv[1]+7;
        host_end = host_start;
        while (*host_end) {
            if (*host_end == '/' || *host_end == ':') {
                break;
            }
            host_end++;
        }
        strncpy(host, host_start, MIN(host_end - host_start, 255));
    }
    char port_str[] = {'8', '0', '\0', '\0', '\0', '\0'};
    char *port_start = host_end;
    char *port_end = port_start;
    if (*host_end == ':') {
        char *port_start = host_end + 1;
        port_end = port_start;
        while (*port_end && *port_end != '/') {
            port_end++;
        }

        size_t port_len = port_end - port_start;
        if (port_len < 1 || port_len > 5) {
            fatal("Invalid port number");
        }
        strncpy(port_str, port_start, port_len);
    }

    char *path_start = port_end;

    struct addrinfo hints = {
        .ai_family = AF_INET,
        .ai_socktype = SOCK_STREAM,
        .ai_protocol = 0,
        .ai_flags = 0,
    };
    struct addrinfo *addr;
    if (getaddrinfo(host, port_str, &hints, &addr) == -1) {
        efatal("getaddrinfo");
    }
    if (addr == 0) {
        printf("Couldn't resolve %s\n", host);
        return 1;
    }

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        efatal("sock");
    }
    if (connect(sock, addr->ai_addr, addr->ai_addrlen) == -1) {
        efatal("connect");
    }

    request(sock, path_start);

    freeaddrinfo(addr);
    return 0;
}
