

KERNELDIR  :=/lib/modules/$(shell  uname  -r)/build
#KERNELDIR :=/home/linux/Exynos4412/linux-3.14-fs4412


test:
	make   -C   $(KERNELDIR)   M=$(shell  pwd)     modules
clean:
	make   -C   $(KERNELDIR)   M=$(shell  pwd)     clean  



obj-m  += hello.o


