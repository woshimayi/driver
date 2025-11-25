

KERNELDIR  :=/lib/modules/$(shell  uname  -r)/build


test:
	make   -C   $(KERNELDIR)   M=$(shell  pwd)     modules
clean:
	make   -C   $(KERNELDIR)   M=$(shell  pwd)     clean  



obj-m  += hello.o


