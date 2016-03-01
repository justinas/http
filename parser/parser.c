#include <stddef.h>
#include <string.h>

#include "parser.h"

// Private declarations
int parser_parse_step(parser *p);
void parser_rotate_buf(parser *p, size_t offset);

http_request* request_new() {
    http_request* req = malloc(sizeof(http_request));
    req->method = Nil;
    req->path[0] = '\0';

    return req;
}

void request_destroy(http_request* req) {
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

    return parser_parse_step(p);
}

// Return values match the ones documented for parser_feed()
int parser_parse_step(parser *p) {
    for (;;) {
        switch (p->state) {
            case Init:
                {
                    if (p->buf_len < 4) {
                        return 0;
                        break;
                    }
                    if (memcmp("GET ", p->buf, 4) != 0) {
                        return -1;
                        break;
                    }

                    p->req->method = Get;
                    parser_rotate_buf(p, 4);

                    p->state = AfterMethod;
                    break;
                }
            case AfterMethod:
                {
                    char *end_of_path = memchr(p->buf, ' ', p->buf_len);
                    if (end_of_path == NULL) {
                        return 0;
                    }

                    size_t len = end_of_path - p->buf;
                    if (len > REQUEST_PATH_SIZE-1) {
                        return -1;
                    }
                    memcpy(p->req->path, p->buf, len);
                    p->req->path[len] = '\0';

                    parser_rotate_buf(p, len);
                    p->state = AfterPath;
                    break;
                }
            default:
                return 1;
        }
    }
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
