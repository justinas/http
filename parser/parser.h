#include <stdlib.h>

#ifndef PARSER_H
#define PARSER_H

#define PARSER_BUF_SIZE 8192
#define REQUEST_HEADER_COMPONENT_SIZE 1024
#define REQUEST_PATH_SIZE 1024

typedef enum http_method {
    Nil,
    Get,
} http_method;

typedef struct http_header http_header;

struct http_header {
    char name[REQUEST_HEADER_COMPONENT_SIZE];
    char value[REQUEST_HEADER_COMPONENT_SIZE];
    http_header *next;
};

typedef struct http_request {
    http_method method;
    char path[REQUEST_PATH_SIZE]; // NUL-terminated
    http_header *header; // A linked list of headers
} http_request;

http_request* request_new();
void request_destroy(http_request* req);

typedef enum parser_state {
    Init,
    AfterMethod,
    AfterPath,
    MaybeHeaderName,
    HeaderValue,
    End,
} parser_state;

typedef struct parser {
    parser_state state;
    char buf[PARSER_BUF_SIZE];
    size_t buf_len;
    http_request *req;
    http_header **current_header;
} parser;

parser* parser_new();
void parser_destroy(parser *p);
// Returns:
//   -1 if invalid syntax has been encountered,
//    0 if the parser expects more data to be fed,
//    1 if the request has been fully parsed.
int parser_feed(parser *p, char *buf, size_t n);
// Returns a pointer to a parsed request.
// Only valid after parser_feed() returns 1.
// The request is destroyed together with the parser
// upon calling parser_destroy().
http_request* parser_request(parser *p);

#endif // PARSER_H
