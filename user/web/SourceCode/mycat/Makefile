include ../Make.defines

CFLAGS    = $(CFLAGSO) -lm
# need -O in this directory, since we're timing the
# programs and we need the getc() and putf() macros
# expanded efficiently in-line.

PROGS =    fgetsfputs getcputc mycat eff

all:    ${PROGS}

clean:
	rm -f ${PROGS} ${TEMPFILES}
