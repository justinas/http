CFLAGS = -g -O0 -std=c99 -Wall

server: ../parser/libparser.a bytebuf.o handler.o main.o
	$(CC) $(CFLAGS) -L../parser -o $@ $^ -lparser 

../parser/libparser.a:
	cd ../parser && make

clean:
	rm -f *.o
	rm -f server
