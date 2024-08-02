CC=gcc
#CC=/opt/toolchains//crosstools-arm-gcc-5.5-linux-4.1-glibc-2.26-binutils-2.28.1/usr/bin/arm-buildroot-linux-gnueabi-gcc

LDFLAGS=
CFLAGS=-g -Wall -Wextra -pedantic

VERSION=0.1

OBJS=main.o tcp.o

all: tcping

tcping: $(OBJS)
	$(CC) $(OBJS) $(LDFLAGS) -o tcping

install: tcping
	cp tcping $(DESTDIR)/usr/bin

clean:
	rm -f $(OBJS) core tcping

package: clean
	# source package
	rm -rf tcping-$(VERSION)*
	mkdir tcping-$(VERSION)
	cp *.c *.h Makefile readme.txt license.txt tcping-$(VERSION)
	tar czf tcping-$(VERSION).tgz tcping-$(VERSION)
	rm -rf tcping-$(VERSION)
