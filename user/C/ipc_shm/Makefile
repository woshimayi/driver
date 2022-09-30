.PHONE: all clean

all: server client

client:client.c comm.c
	gcc -g -o $@ $^

server:server.c comm.c
	gcc -g -o $@ $^

clean:
	rm -rf client serer
