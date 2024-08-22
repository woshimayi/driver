.PHONE: all clean

all: server client

client:mmap_client.c
	gcc -g -o $@ $^

server:mmap_server.c
	gcc -g -o $@ $^

clean:
	rm -rf client serer
