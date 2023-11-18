
# all: client server
.PHONY: client server info

client:
	gcc -o client ./convert_client.c && ./client 127.0.0.1 12345

server: 
	gcc -o server ./convert_server.c && ./server 12345

multi_server:
	gcc -o server ./multiplex_server.c && ./server 12345

multi_client:
	gcc -o client ./multiplex_client.c && ./client 127.0.0.1 12345

add_server:
	gcc -o server ./add_server.c && ./server 12345

add_client:
	gcc -o client ./add_client.c && ./client 127.0.0.1 12345

convert_multiplex_server:
	gcc -o server ./convert_multiplex_server.c && ./server 12345

convert_multiplex_client:
	gcc -o client ./convert_multiplex_client.c && ./client 127.0.0.1 12345

info:
	gcc -o info ./test.c && ./info