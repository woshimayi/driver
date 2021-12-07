CFLAGS = -DLINUX

CC=gcc

SHARED_FLAG = -fPIC -shared



INC += -I.

# SOURCES = $(foreach d,.,$(wildcard $(d)/*.c))
SOURCES = $(wildcard *.c)
OBJS = $(patsubst %.c, %.o, $(SOURCES))
OBJS += crchash.o

%.o : %.c
	$(CC) $(INC) -c $< -o $@

CC += $(SHARED_FLAG)

all : libcrc.a
	@echo "SOURCES"
	@echo $(SOURCES)
	@echo "OBJS"
	@echo $(OBJS)
	@echo $@ $<

libcrc.a : $(OBJS)
	ar -rs $@ $^


clean:
	rm -rf *.o *.a

