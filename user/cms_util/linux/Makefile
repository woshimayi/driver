LIB  := oal.a

OBJS := oal_logging.o oal_readlog.o oal_memory.o oal_timestamp.o \
       oal_strconv.o oal_pid.o \
       oal_prctl.o oal_network.o oal_file.o oal_passwd.o ty_parse_dns.o


all install: $(LIB)

clean:
	rm -f *.o *.d $(LIB)


$(LIB) : $(OBJS)
	$(AR) -rcs $@ $(OBJS)


# Set VPATH because we want to compile in a seperate dir than source.
name := $(lastword $(MAKEFILE_LIST))
mdir := $(realpath $(dir $(name)))
VPATH=$(mdir)

# Generate and use dependencies.
CFLAGS += -g -MD
-include $(OBJS:%.o=%.d)

CURR_DIR := $(shell pwd)
BUILD_DIR:=$(subst /userspace, /userspace,$(CURR_DIR))
BUILD_DIR:=$(word 1, $(BUILD_DIR))

include $(BUILD_DIR)/make.common

ALLOWED_INCLUDE_PATHS := -I. -I..\
                         -I$(BUILD_DIR)/userspace/public/include  \
                         -I$(BUILD_DIR)/userspace/public/include/$(OALDIR) \
                         -I$(BUILD_DIR)/userspace/private/include  \
                         -I$(BUILD_DIR)/userspace/private/include/$(OALDIR)

ALLOWED_INCLUDE_PATHS += -I$(BUILD_DIR)/userspace/public/libs/bcm_util

ALLOWED_INCLUDE_PATHS += -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD)  \
                         -I$(INC_BRCMDRIVER_PRIV_PATH)/$(BRCM_BOARD) \
                         -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD)

ALLOWED_INCLUDE_PATHS += -I$(KERNEL_LINKS_DIR)
