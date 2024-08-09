

KERNEL_DIR=/usr/src/linux-headers-$(shell uname -r)
obj-m += netlink_test.o
ccflags-y := -std=gnu99 -Wno-declaration-after-statement

all:
	$(CC) nl_recv.c -o nl_recv
	make -C $(KERNEL_DIR) M=$(PWD) modules

clean:
	rm -f nl_recv
	make -C $(KERNEL_DIR) M=$(PWD) clean
