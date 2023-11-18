
# all: client server
.PHONY: client server

client:
	gcc -o client ./client.c && ./client 127.0.0.1 12345

server: 
	gcc -o server ./server.c && ./server 12345
