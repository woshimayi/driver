include ../Make.defines

PROGS =    access cdpwd changemod conf devrdev \
	fileflags filetype ftw4 \
	hello hole longpath ls1 mycd \
	seek testerror \
	uidgid umask unlink zap

all:    ${PROGS}

savedid: savedid.o
	$(LINK.c) -o savedid savedid.o $(LDLIBS) $(S5LIB)

clean:
	rm -f ${PROGS} ${TEMPFILES} file.hole
