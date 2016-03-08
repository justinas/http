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

    // Real life test-case: Safari
    {
        char *req_str = "GET / HTTP/1.1\r\n"
            "Host: 127.0.0.1:8000\r\n"
            "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n"
            "Cookie: __utma=96992031.1037738645.1429089370.1456747257.1457013590.3; __utmz=96992031.1456747257.2.1.utmcsr=(direct)|utmccn=(direct)|utmcmd=(none); __zlcmid=UGeZzV9tNTB3oY; csrftoken=MszhuwDmnu3fcWooBlotV7bfst347MnU; newsletter_popup_dismissed=1\r\n"
            "User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_10_5) AppleWebKit/601.4.4 (KHTML, like Gecko) Version/9.0.3 Safari/601.4.4\r\n"
            "Accept-Language: en-us\r\n"
            "Accept-Encoding: gzip, deflate\r\n"
            "Connection: keep-alive\r\n"
            "\r\n";
        parser *p = parser_new();
        assert(parser_feed(p, req_str, 128) == 0);
        assert(parser_feed(p, req_str+128, strlen(req_str+128)) == 1);

        http_request *req = parser_request(p);

        http_header *h = req->header;
        assert(h);
        assert(strcmp(h->name, "Host") == 0);
        assert(strcmp(h->value, "127.0.0.1:8000") == 0);

        h = h->next;
        assert(h);
        assert(strcmp(h->name, "Accept") == 0);
        assert(strcmp(h->value, "text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8") == 0);

        h = h->next;
        assert(h);
        assert(strcmp(h->name, "Cookie") == 0);
        assert(strcmp(h->value, "__utma=96992031.1037738645.1429089370.1456747257.1457013590.3; __utmz=96992031.1456747257.2.1.utmcsr=(direct)|utmccn=(direct)|utmcmd=(none); __zlcmid=UGeZzV9tNTB3oY; csrftoken=MszhuwDmnu3fcWooBlotV7bfst347MnU; newsletter_popup_dismissed=1") == 0);

        parser_destroy(p);
    }
}

int main() {
    test_rotate_buf();
    test_parser_feed();
    return 0;
}
