KVERS = $(shell uname -r)

obj-m += char_dev.o

build: kernel_modules test

kernel_modules:
	make -C /lib/modules/$(KVERS)/build M=$(CURDIR) modules
clean:
	make -C /lib/modules/$(KVERS)/build M=$(CURDIR) clean
	rm test
