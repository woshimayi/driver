ccflags-y := -std=gnu99 -Wno-declaration-after-statement    # 必须放在子模块中


obj-m += iptables_trace.o

ifneq ($(SUB_BUILD), "SUB")

-include ../config.mk

all:
	make -C $(KER_DIR) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) M=$(PWD) modules

clean:
	make -C $(KER_DIR) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) M=$(PWD) clean

endif




