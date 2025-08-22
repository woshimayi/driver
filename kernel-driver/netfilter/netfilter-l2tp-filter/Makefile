obj-m += l2tp-filter.o
ccflags-y := -std=gnu99 -Wno-declaration-after-statement

CROSS_COMPILE=/home/zhengsen/sdk/fttr-/fttr/tmp/tiangong0_cmcc_hgu_turbo/tools/cross_toolchain/arm-mix510-linux/arm-mix510-linux/bin/arm-mix510-linux-
KERNEL_DIR=/home/zhengsen/sdk/fttr-/fttr/tmp/tiangong0_cmcc_hgu_turbo/open_source/local/linux/linux-5.10.0/
ARCH=arm

all:
	make -C $(KERNEL_DIR) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) M=$(PWD) modules

clean:
	make -C $(KERNEL_DIR) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) M=$(PWD) clean




