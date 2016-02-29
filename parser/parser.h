#include <stdlib.h>

#ifndef PARSER_H
#define PARSER_H

#define PARSER_BUF_SIZE 8192

typedef enum http_method {
    Nil,
    Get,
} http_method;

typedef struct http_request {
    http_method method;
    char *path; // NUL-terminated
} http_request;

http_request* request_new();
void request_destroy(http_request* req);

typedef enum parser_state {
    Init,
    AfterMethod,
} parser_state;

typedef struct parser {
    parser_state state;
    char buf[PARSER_BUF_SIZE];
    size_t buf_len;
    http_request *req;
} parser;

parser* parser_new();
void parser_destroy(parser *p);
// Returns:
//   -1 if invalid syntax has been encountered,
//    0 if the parser expects more data to be fed,
//    1 if the request has been fully parsed.
int parser_feed(parser *p, char *buf, size_t n);

#endif // PARSER_H
