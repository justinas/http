#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/stat.h>

#include "../parser/parser.h"

#include "bytebuf.h"
#include "client.h"


bytebuf* handler(client* cl) {
    http_request *req = parser_request(cl->parser);

    bytebuf* response = bytebuf_new();
    bytebuf_append_str(response, "HTTP/1.0 200 OK\r\n\r\n");

    char *path = req->path;
    if (strcmp(path, "/") == 0) {
        char wd[256];
        if (!getcwd(wd, sizeof wd)) {
            perror("getcwd");
            return response;
        }
        DIR *dir = opendir(wd);
        if (!dir) {
            perror("opendir");
        }
        struct dirent *entry;

        char *init_html = "<!doctype html><title>Server</title>\n";
        bytebuf_append_str(response, init_html);
     
        bytebuf_append_str(response, "<h2>Files</h2>");
        bytebuf_append_str(response, "<table>");
        while ((entry = readdir(dir))) {
            struct stat info;
            if (stat(entry->d_name, &info) < 0) {
                perror("stat");
            }
            char size_buf[30];
            snprintf(size_buf, sizeof size_buf,
                    "%lu bytes", info.st_size);
            bytebuf_append_str(response, "<tr><td>");

            bytebuf_append_str(response, "<a href=\"");
            bytebuf_append_str(response, entry->d_name);
            bytebuf_append_str(response, "\">");
            bytebuf_append_str(response, entry->d_name);
            bytebuf_append_str(response, "</a>");

            bytebuf_append_str(response, "</td><td>");
            bytebuf_append_str(response, size_buf);
            bytebuf_append_str(response, "</td></tr>");
        }
        bytebuf_append_str(response, "</table>");

        bytebuf_append_str(response, "<h2>Headers</h2>");
        bytebuf_append_str(response, "<table>");
        for (http_header *h = req->header; h != NULL; h = h->next) {
            bytebuf_append_str(response, "<tr><td>");
            bytebuf_append_str(response, h->name);
            bytebuf_append_str(response, "</td><td>");
            bytebuf_append_str(response, h->value);
            bytebuf_append_str(response, "</td></tr>");
        }
        bytebuf_append_str(response, "</table>");
    }
    else {
        assert(path[0] == '/');
        path++;
        FILE *f = fopen(path, "r");
        if (!f) {
            bytebuf_append_str(response, strerror(errno));
            return response;
        }

        char buf[1024];
        size_t n;
        while((n = fread(&buf, 1, 1024, f)) != 0) {
            bytebuf_append(response, buf, n);
        }
        fclose(f);
    }

    return response;
}
