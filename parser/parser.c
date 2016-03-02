#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "parser.h"

// Private declarations
int parser_parse_step(parser *p);
void parser_rotate_buf(parser *p, size_t offset);

http_request* request_new() {
    http_request* req = malloc(sizeof(http_request));
    req->method = Nil;
    req->path[0] = '\0';
    req->header = NULL;

    return req;
}

void request_destroy(http_request* req) {
    free(req);

    http_header *h = req->header;
    while (h != NULL) {
        http_header *tmp = h;
        h = tmp->next;
        free(tmp);
    }
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
    http_header **current_header = &p->req->header;
    char header_name[REQUEST_HEADER_COMPONENT_SIZE];
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

                    parser_rotate_buf(p, len+1);
                    p->state = AfterPath;
                    break;
                }
            case AfterPath:
                {
                    // strlen("HTTP/1.1\r\n") == 10
                    if (p->buf_len < 10) {
                        return 0;
                    }
                    if (memcmp(p->buf, "HTTP/1.", 7) != 0 ||
                            !isdigit(p->buf[7]) ||
                            p->buf[8] != '\r' ||
                            p->buf[9] != '\n') {
                        return -1;
                    }
                    parser_rotate_buf(p, 10);
                    p->state = MaybeHeaderName;
                    break;
                }
            case MaybeHeaderName:
                {
                    if (p->buf_len >= 2 &&
                            p->buf[0] == '\r' &&
                            p->buf[1] == '\n') {
                        p->state = End;
                        break;
                    }

                    char *end_of_name;
                    end_of_name = memchr(p->buf, ':', p->buf_len);
                    if (end_of_name == NULL) {
                        return 0;
                    }
                    if (*(end_of_name+1) != ' ') {
                        return -1;
                    }
                    size_t len = (size_t) (end_of_name - p->buf);
                    if (len > REQUEST_HEADER_COMPONENT_SIZE-1) {
                        return -1;
                    }
                    memcpy(header_name, p->buf, len);
                    header_name[len] = '\0';
                    parser_rotate_buf(p, len+2);
                    p->state = HeaderValue;
                }
            case HeaderValue:
                {
                    char *end_of_value = memchr(p->buf, '\r', p->buf_len);
                    if (end_of_value == NULL) {
                        if (p->buf_len > REQUEST_HEADER_COMPONENT_SIZE+2) {
                            return -1;
                        }
                        return 0;
                    }

                    // nothing received after \r yet
                    if (end_of_value == (p->buf + p->buf_len)) {
                        return 0;
                    }
                    if (*(end_of_value+1) != '\n') {
                        return -1;
                    }
                    size_t len = end_of_value - p->buf;
                    if (len > REQUEST_HEADER_COMPONENT_SIZE-1) {
                        return -1;
                    }
                    *current_header = malloc(sizeof(http_header));
                    strcpy((*current_header)->name, header_name);
                    memcpy((*current_header)->value, p->buf, len);
                    (*current_header)->value[len] = '\0';
                    current_header = &(*current_header)->next;

                    parser_rotate_buf(p, len+2);
                    p->state = MaybeHeaderName;
                    break;
                }
            case End:
                return 1;
            default:
                abort();
        }
    }
}

http_request* parser_request(parser *p) {
    return p->req;
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
