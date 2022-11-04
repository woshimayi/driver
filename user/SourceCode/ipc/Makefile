.PHONE: all clean

all: server client

client:client.c comm.c
        gcc -o $@ $^

server: server.c comm.c
        gcc o $@ $^

clean:
        rm -rf client serer