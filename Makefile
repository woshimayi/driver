obj-m := hello.o

KERNELDIR:=/lib/modules/$(shell uname -r)/build

PWD:=$(shell pwd)



all:
	make -C $(KERNELDIR) M=$(PWD) modules



clean:
	rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c .tmp_versions modules.order  Module.symvers
