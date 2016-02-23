CFLAGS = -g -std=c99 -Wall

server: main.o
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f *.o
	rm -f server
