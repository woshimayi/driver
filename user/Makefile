ifeq ($(strip $(HEAD)), y)

$(warning $(HEAD))
CFLAGS += -DHEAD_1

else

$(warning $(HEAD))
CFLAGS += -DHEAD_2

endif

$(warning $(CFLAGS))

CC = gcc

all: main.o
	$(CC) $^ -o $@ $(CFLAGS)
	
clean:
	rm main main.o all

