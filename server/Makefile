CFLAGS = -g -std=c99 -Wall

server: main.o
	$(CC) $(CFLAGS) -L../parser -lparser -o $@ $<

clean:
	rm -f *.o
	rm -f server
