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
    // Test method parsing
    {
        char *req_str = "GET ";
        parser *p = parser_new();
        assert(parser_feed(p, req_str, strlen(req_str)) == 0);
        assert(p->req->method == Get);
        parser_destroy(p);
    }

    {
        char *req_str = "POST ";
        parser *p = parser_new();
        assert(parser_feed(p, req_str, strlen(req_str)) == -1);
        assert(p->req->method == Nil);
        parser_destroy(p);
    }

    // Test path parsing
    {
        char *req_str = "GET /really/long/path/with/slashes ";
        parser *p = parser_new();
        assert(parser_feed(p, req_str, strlen(req_str)) == 0);
        assert(p->req->method == Get);
        assert(strcmp(p->req->path, "/really/long/path/with/slashes") == 0);
        parser_destroy(p);
    }

    // Test version parsing
    {
        char *req_str = "GET /really/long/path/with/slashes HTTP/1.1\r\n";
        parser *p = parser_new();
        assert(parser_feed(p, req_str, strlen(req_str)) == 0);
        parser_destroy(p);
    }

    {
        char *req_str = "GET /really/long/path/with/slashes HTTP/1.x\r\n";
        parser *p = parser_new();
        assert(parser_feed(p, req_str, strlen(req_str)) == -1);
        parser_destroy(p);
    }

    // Test header parsing
    {
        char *req_str = "GET /really/long/path/with/slashes HTTP/1.1\r\n"
            "Host: localhost\r\n"
            "\r\n";
        parser *p = parser_new();
        assert(parser_feed(p, req_str, strlen(req_str)) == 1);
        parser_destroy(p);
    }

    {
        char *req_str = "GET /really/long/path/with/slashes HTTP/1.1\r\n" 
            "Accept: Anything\r\n"
            "Host: localhost\r\n"
            "\r\n";
        parser *p = parser_new();
        assert(parser_feed(p, req_str, strlen(req_str)) == 1);
        http_request *req = parser_request(p);

        http_header *h = req->header;
        assert(h);
        assert(strcmp(h->name, "Accept") == 0);
        assert(strcmp(h->value, "Anything") == 0);

        h = h->next;
        assert(h);
        assert(strcmp(h->name, "Host") == 0);
        assert(strcmp(h->value, "localhost") == 0);

        h = h->next;
        assert(!h);
        parser_destroy(p);
    }
}

int main() {
    test_rotate_buf();
    test_parser_feed();
    return 0;
}
