KERNELDIR := ../../linux-imx-4.1.15-2.1.0
CURRENT_PATH := $(shell pwd)
INSTALL_DIR := /home/zs/linux/nfs/rootfs/lib/modules/4.1.15/
CC := arm-linux-gnueabihf-gcc
sub := $(shell find -name *App.c)
# get file name
obj := $(basename $(notdir $(sub)))

obj-m := atm24c02.o


$(warning $(obj-m))

build: kernel_modules  $(obj)

kernel_modules:
	$(MAKE) -C $(KERNELDIR) M=$(CURRENT_PATH) modules
 
$(obj): $(obj-m)
	$(CC)  $(sub)  -o  $(obj) -lm

install: 
	install $(CURRENT_PATH)/*.ko  $(INSTALL_DIR)
	install $(CURRENT_PATH)/*App  $(INSTALL_DIR)

clean:
	$(MAKE) -C $(KERNELDIR) M=$(CURRENT_PATH) clean	
	rm $(obj)
