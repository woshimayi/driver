CFLAGS = -DLINUX

CC=gcc

SHARED_FLAG = -fPIC -shared



INC += -I.

CC += $(SHARED_FLAG)

object = crc16.o crc32.o crc64.o 

lib: $(object)
	$(CC) -o lib $(object)

$(object): crc16.c crc16.h crc32.c crc32.h crc64.c crc64.h
	$(CC) $(INC) -c ./%.c -o ./%.o


.PHONE: clean
clean:
	rm -rf *.o lib