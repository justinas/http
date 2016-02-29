#include <stddef.h>
#include <string.h>

#include "parser.h"

http_request* request_new() {
    http_request* req = malloc(sizeof(http_request));
    req->method = Nil;
    req->path = NULL;

    return req;
}

void request_destroy(http_request* req) {
    free(req->path);
    free(req);
}

parser* parser_new() {
    parser *instance = malloc(sizeof(parser));
    instance->state = Init;
    instance->buf_len = 0;
    instance->req = request_new();

    return instance;
}

void parser_destroy(parser *p) {
    request_destroy(p->req);
    free(p);
}

int parser_feed(parser *p, char *buf, size_t n) {
    if (n > (PARSER_BUF_SIZE - p->buf_len)) {
        abort();
    }

    memcpy(p->buf + p->buf_len, buf, n);
    p->buf_len += n;

    return 0;
}

int parser_parse_step(parser *p) {
    return 0;
}

void parser_rotate_buf(parser *p, size_t offset) {
    if (offset == 0) {
        return;
    }

    size_t bound = p->buf_len - offset;
    for (size_t i = 0; i < bound; i++) {
        p->buf[i] = p->buf[i+offset];
    }
    p->buf_len -= offset;
}
