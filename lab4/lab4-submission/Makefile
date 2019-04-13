
all: client server

client: client.c
	gcc -g -Wall -lpthread -o client client.c

server: server.c
	gcc -g -Wall -Werror -lpthread -o server server.c

clean:
	rm -rf client server