KVERS = $(shell uname -r)
KVERS = "6.11.0-21-generic"

obj-m += kprobe_test.o

build: kernel_modules

kernel_modules:
	make -C /lib/modules/$(KVERS)/build M=$(CURDIR) modules

clean:
	make -C /lib/modules/$(KVERS)/build M=$(CURDIR) clean
