CPPFLAGS += 
LDLIBS += 

all: hello

analyzestack: hello.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

clean:
	rm -f *.o hello

.PHONE: all clean