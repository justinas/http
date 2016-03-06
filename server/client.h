#include <stdbool.h>

#include "../parser/parser.h"

#include "bytebuf.h"

#ifndef CLIENT_H
#define CLIENT_H

typedef struct client client;

struct client {
    int fd;
    parser *parser;
    int parser_status;

    bytebuf *response;
    size_t resp_bytes_written;
};

#endif
