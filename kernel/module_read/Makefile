KVERS = $(shell uname -r)

obj-m += char_dev.o

build: kernel_modules

kernel_modules:
	make -C /lib/modules/$(KVERS)/build M=$(CURDIR) modules
	gcc test.c -o test
clean:
	make -C /lib/modules/$(KVERS)/build M=$(CURDIR) clean
	rm test
