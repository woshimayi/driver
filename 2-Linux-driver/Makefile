SUB_DIRS=$(shell ls -d */)




SUBDIRS := $(SUB_DIRS)
CLEAN := $(SUB_DIRS)

all: sub clean



# 循环遍历执行子目录Makefile
sub:$(SUBDIRS)
$(SUBDIRS):ECHO
	@+make -C $@

clean: $(CLEAN)
$(CLEAN):
	@+make -C $@ clean


ECHO:
	@echo "Compiling" $(SUBDIRS)
