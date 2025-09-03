obj-m += fileter_l2tp.o
ccflags-y := -std=gnu99 -Wno-declaration-after-statement

CROSS_COMPILE=arm-mix510-linux-
KER_DIR=linux-5.10.0/

all:
	make -C $(KER_DIR) ARCH=arm CROSS_COMPILE=$(CROSS_COMPILE) M=$(PWD) modules

clean:
	make -C $(KER_DIR) ARCH=arm CROSS_COMPILE=$(CROSS_COMPILE) M=$(PWD) clean




