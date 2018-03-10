all: clean build-server build-client

clean:
	rm -rf server client

build-server: server.c
	gcc server.c -o server

build-client: client.c
	gcc client.c -o client

gcd:
	gcc -Wall -Werror -o gcd-example gcd-example.c
