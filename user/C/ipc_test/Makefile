.PHONE: all clean

all: server client

client:client.c
	gcc -g -o $@ $^

server:server.c
	gcc -g -o $@ $^

clean:
	rm -rf client serer
