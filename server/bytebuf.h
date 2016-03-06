#ifndef BYTEBUF_H
#define BYTEBUF_H

#define BYTEBUF_INITIAL_CAP 128

#define bytebuf_append_str(b, s) bytebuf_append(b, s, strlen(s));

typedef struct bytebuf {
    char *data;
    size_t len;
    size_t cap;
} bytebuf;

bytebuf* bytebuf_new();
void bytebuf_append(bytebuf* b, char *cont, size_t n);
void bytebuf_destroy(bytebuf *b);

#endif
