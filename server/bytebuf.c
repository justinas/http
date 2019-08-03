#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bytebuf.h"

void bytebuf_grow(bytebuf *b);

bytebuf *bytebuf_new() {
    bytebuf *b = malloc(sizeof(bytebuf));
    b->data = malloc(BYTEBUF_INITIAL_CAP);
    if (b->data == NULL) {
        abort();
    }
    b->cap = BYTEBUF_INITIAL_CAP;
    b->len = 0;

    return b;
}

void bytebuf_append(bytebuf *b, char *cont, size_t n) {
    while (b->len + n > b->cap) {
        bytebuf_grow(b);
    }
    memcpy(b->data + b->len, cont, n);
    b->len += n;
}

void bytebuf_grow(bytebuf *b) {
    b->data = realloc(b->data, b->cap * 2);
    if (b->data == NULL) {
        abort();
    }
    b->cap = b->cap * 2;
}

void bytebuf_destroy(bytebuf *b) {
    free(b->data);
    free(b);
}
