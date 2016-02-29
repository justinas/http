CFLAGS = -g -std=c99 -Wall

libparser.a: parser.o
	ar -rc $@ $<

test-bin: libparser.a main.c
	gcc -o $@ -L. -lparser main.c

test: test-bin
	./test-bin

clean:
	rm -f *.o
	rm -f libparser.a
