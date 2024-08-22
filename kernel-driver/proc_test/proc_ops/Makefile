

KERNEL_DIR=/usr/src/linux-headers-$(shell uname -r)
obj-m += proc_test.o
ccflags-y := -std=gnu99 -Wno-declaration-after-statement

all:
	# $(CC) proc_test.c -o proc_test
	make -C /usr/src/linux-headers-$(shell uname -r) M=$(PWD) modules

clean:
	rm -f proc_test
	make -C /usr/src/linux-headers-$(shell uname -r) M=$(PWD) clean
