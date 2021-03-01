KERNELDIR := ../../linux-imx-4.1.15-2.1.0
CURRENT_PATH := $(shell pwd)
obj-m := chrdevbase.o

build: kernel_modules chrdevbaseApp

kernel_modules:
	$(MAKE) -C $(KERNELDIR) M=$(CURRENT_PATH) modules
clean:
	$(MAKE) -C $(KERNELDIR) M=$(CURRENT_PATH) clean
