
LIBTARGET = libdnsparse.so
EXE = test

CSRC = $(wildcard *.c)

# COBJS = $(patsubst %.c, %.o, $(CSRC))
COBJS = dns_parse.c 

EXEOBJS = test.c dns_parse.c 

all: $(LIBTARGET) $(EXE)
	install -m 755 $(LIBTARGET) $(INSTALL_DIR)/lib


ALLOWED_INCLUDE_PATHS := -I. \

CFLAGS += -shared -fPIC  
# treat all warnings as errors
CFLAGS += -Werror -Wfatal-errors

STRIP = strip

$(LIBTARGET): $(COBJS)
	$(CC) -o $@ $^ $(ALLOWED_INCLUDE_PATHS) $(CFLAGS) $(LIBS) $(LIB_DIR)
	$(STRIP) $@

$(EXE): $(EXEOBJS)
	$(CC) -o $@ $^ $(ALLOWED_INCLUDE_PATHS) $(LIBS) $(LIB_DIR)
	$(STRIP) $@


clean:
	-rm -f *.o *.d *.d.* *.so $(LIBTARGET)

.PHONY: all clean


$(COBJS):%.o:%.c
	$(CC) $(CFLAGS) $< $(ALLOWED_INCLUDE_PATHS) -c -o $@


%.d: %.c
	@set -e; rm -f $@; \
	$(CC) -M $(CPPFLAGS) $(CFLAGS) $< > $@.$$$$ 2>/dev/null; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$
