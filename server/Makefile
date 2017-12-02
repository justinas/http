CFLAGS = -g -O0 -std=c99 -Wall

server: bytebuf.o handler.o main.o
	$(CC) $(CFLAGS) -L../parser -o $@ $^ -lparser 

clean:
	rm -f *.o
	rm -f server
