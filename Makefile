.PHONY: all client server
all: client server

client:
	cd client && make
server:
	cd server && make
