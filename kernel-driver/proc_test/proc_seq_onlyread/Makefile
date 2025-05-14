

MOD_NAME := proc_seq
KERNEL_DIR=/usr/src/linux-headers-$(shell uname -r)
obj-m += $(MOD_NAME).o
ccflags-y := -std=gnu99 -Wno-declaration-after-statement

all:
	make -C /usr/src/linux-headers-$(shell uname -r) M=$(PWD) modules

clean:
	make -C /usr/src/linux-headers-$(shell uname -r) M=$(PWD) clean
