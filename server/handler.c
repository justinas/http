#include <stdlib.h>
#include <string.h>

#include "../parser/parser.h"

#include "bytebuf.h"
#include "client.h"

bytebuf* handler(client* cl) {
    http_request *req = parser_request(cl->parser);

    bytebuf* response = bytebuf_new();

    char *msg = "HTTP/1.0 200 OK\r\n\r\n";
    bytebuf_append_str(response, msg);
    char *init_html = "<!doctype html><title>Server</title>\n"
        "<table>";
    bytebuf_append_str(response, init_html);

    for (http_header *h = req->header; h != NULL; h = h->next) {
        bytebuf_append_str(response, "<tr><td>");
        bytebuf_append_str(response, h->name);
        bytebuf_append_str(response, "</td><td>");
        bytebuf_append_str(response, h->value);
        bytebuf_append_str(response, "</td></tr>");
    }
    bytebuf_append_str(response, "</table>");

    return response;
}
