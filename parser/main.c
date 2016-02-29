#include <assert.h>
#include <string.h>

#include "parser.h"

void parser_rotate_buf(parser *p, size_t offset);

void test_rotate_buf() {
    parser *p = parser_new();
    strcpy(p->buf, "abcdef");
    p->buf_len = 6;
    parser_rotate_buf(p, 3);
    assert(p->buf[0] == 'd');
    assert(p->buf[1] == 'e');
    assert(p->buf[2] == 'f');
    assert(p->buf_len == 3);
}

void test_parser_feed() {
    char *req_str = "GET /";
    parser *p = parser_new();
    parser_feed(p, req_str, strlen(req_str));
    assert(p->buf_len == 5);
    assert(memcmp(req_str, p->buf, 5) == 0);
    parser_destroy(p);
}

int main() {
    test_rotate_buf();
    test_parser_feed();
    return 0;
}
