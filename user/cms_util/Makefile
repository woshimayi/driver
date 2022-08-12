LIB  := libfwk.so

OBJS := eid.o logging.o memory.o bget.o psp.o timestamp.o timer.o \
       eid_parser.o strconv.o prctl.o \
       assert.o network.o file.o \
       base64_wrapper.o hexbinary_wrapper.o \
       lzw_encode.o lzw_decode.o xml.o led.o passwd.o \
       image.o \
       tokenizer.o errorcodes.o \
       ftrace.o linklist.o random.o unicode.o \
       pcp.o data_model_selector.o \
       strconv2.o math.o md5.o \
       dhcp_common.o gb_utf8_convert.o strconv_hg.o
       
# if you want to use CMS_MEM_LEAK_TRACING feature on ARM based platforms,
# uncomment out the following line.  But to avoid complicated license
# issues, do NOT compile this file into your production code.
# OBJS += backtrace_arm.o


# image.c needs to know what chip we are using
CFLAGS += -Werror -Wfatal-errors

# OS abstraction layer
OALDIR := linux

MODDIR = userspace/public/libs/cms_util

# EID symbol table
SYMBOL_TAB := symbol_table.txt
REALTIME_LEVELS := realtime_levels.txt

# EID files in the scripts directory
EID_FILES = eid_bcm_base.txt \
            eid_bcm_kthreads.txt \
            eid_bcm_mgmt.txt \
            eid_bcm_voip.txt

all: $(LIB) $(SYMBOL_TAB) generic_public_lib_install

install: all
	mkdir -p $(LIB_INSTALL_DIR)
	install -t $(LIB_INSTALL_DIR) $(LIB)
	mkdir -p $(ETC_INSTALL_DIR)
	install -p -m 444 -t $(ETC_INSTALL_DIR) $(SYMBOL_TAB) $(addprefix $(mdir)/$(MODDIR)/scripts/,$(EID_FILES))


CURR_DIR := $(shell pwd)
BUILD_DIR:=$(subst /userspace, /userspace,$(CURR_DIR))
BUILD_DIR:=$(word 1, $(BUILD_DIR))

include $(BUILD_DIR)/make.common

ALLOWED_INCLUDE_PATHS := -I.\
                         -I$(BCM_FSBUILD_DIR)/public/include \
                         -I$(BCM_FSBUILD_DIR)/private/include \
                         -I$(BUILD_DIR)/userspace/public/include  \
                         -I$(BUILD_DIR)/userspace/public/include/$(OALDIR)                                             

ifneq ($(strip $(BUILD_OMCI)),)
ALLOWED_INCLUDE_PATHS += -I$(BCM_FSBUILD_DIR)/private/include
endif

ALLOWED_INCLUDE_PATHS += -I$(INC_BRCMDRIVER_PUB_PATH)/$(BRCM_BOARD)  \
                         -I$(INC_BRCMDRIVER_PRIV_PATH)/$(BRCM_BOARD) \
                         -I$(INC_BRCMSHARED_PUB_PATH)/$(BRCM_BOARD) \
                         -I$(BCM_FSBUILD_DIR)/public/include

clean:
	rm -f *.o *.d $(LIB)
	rm -f $(OALDIR)/oal.a $(OALDIR)/*.o $(OALDIR)/*.d
	rm -f $(SYMBOL_TAB) $(REALTIME_LEVELS)
	rm -f $(LIB_INSTALL_DIR)/$(LIB)
	rm -f $(addprefix $(ETC_INSTALL_DIR)/,$(EID_FILES))
	rm -f $(ETC_INSTALL_DIR)/$(SYMBOL_TAB)


ifneq (,$(wildcard $(KERNEL_DIR)/include/linux/bcm_realtime.h))
$(REALTIME_LEVELS):
	@grep '#define BCM_RTPRIO_' $(KERNEL_DIR)/include/linux/bcm_realtime.h | sed -e 's/#define//g' > $@
else
# Look for bcm_realtime.h in its new location in new kernel.
# All this stuff will be moved out of CMS for 5.04L.01.
$(REALTIME_LEVELS):
	@grep '#define BCM_RTPRIO_' $(BUILD_DIR)/kernel/bcmkernel/include/uapi/linux/bcm_realtime.h | sed -e 's/#define//g' > $@
endif

# grab some #defines from cms_eid.h and cms_params.h so that symbols can
# be used in the eid files
SYMBOL_TABLE_SRCS := $(BUILD_DIR)/userspace/public/include/cms_eid.h \
                     $(BUILD_DIR)/userspace/public/include/cms_params.h \
                     $(KERNEL_DIR)/include/uapi/linux/sched.h \
                     $(REALTIME_LEVELS)

$(warning "zzzzz", $@)
$(SYMBOL_TAB): $(SYMBOL_TABLE_SRCS)
	@echo Generating $@
	@egrep 'EID_[[:alnum:]_-]+=' $(BUILD_DIR)/userspace/public/include/cms_eid.h | sed -e 's/,//g' | sed -e 's/=/ /g' > $@
	@egrep 'NDA_ACCESS_[[:alnum:]_\t\ ]+0x' $(BUILD_DIR)/userspace/public/include/cms_eid.h | sed -e 's/#define//g' >> $@
	@grep '#define EIF_' $(BUILD_DIR)/userspace/public/include/cms_eid.h | sed -e 's/#define//g' >> $@
	@grep '#define FTPD_PORT' $(BUILD_DIR)/userspace/public/include/cms_params.h | sed -e 's/#define//g' >> $@
	@grep '#define TFTPD_PORT' $(BUILD_DIR)/userspace/public/include/cms_params.h | sed -e 's/#define//g' >> $@
	@grep '#define SSHD_PORT' $(BUILD_DIR)/userspace/public/include/cms_params.h | sed -e 's/#define//g' >> $@
	@grep '#define TELNETD_PORT' $(BUILD_DIR)/userspace/public/include/cms_params.h | sed -e 's/#define//g' >> $@
	@grep '#define HTTPD_PORT' $(BUILD_DIR)/userspace/public/include/cms_params.h | sed -e 's/#define//g' >> $@
	@grep '#define SNMPD_PORT' $(BUILD_DIR)/userspace/public/include/cms_params.h | sed -e 's/#define//g' >> $@
	@grep '#define TR69C_CONN_REQ_PORT' $(BUILD_DIR)/userspace/public/include/cms_params.h | sed -e 's/#define//g' >> $@
	@grep '#define BUCPE_CONN_REQ_PORT' $(BUILD_DIR)/userspace/public/include/cms_params.h | sed -e 's/#define//g' >> $@
	@grep '#define TR64C_HTTP_CONN_PORT' $(BUILD_DIR)/userspace/public/include/cms_params.h | sed -e 's/#define//g' >> $@
	@grep '#define TR69C_2_CONN_REQ_PORT' $(BUILD_DIR)/userspace/public/include/cms_params.h | sed -e 's/#define//g' >> $@
	@grep '#define SCHED_NORMAL' $(KERNEL_DIR)/include/uapi/linux/sched.h | sed -e 's/#define//g' >> $@
	@grep '#define SCHED_FIFO' $(KERNEL_DIR)/include/uapi/linux/sched.h | sed -e 's/#define//g' >> $@
	@grep '#define SCHED_RR' $(KERNEL_DIR)/include/uapi/linux/sched.h | sed -e 's/#define//g' >> $@
	@cat $(REALTIME_LEVELS) >> $@


$(LIB): $(OBJS) force_oal
	$(CC) -g -O0 -shared $(BCM_LD_FLAGS) -Wl,--whole-archive,-soname,$(notdir $@) \
	    -o $@ $(OBJS) $(OALDIR)/oal.a -Wl,-lcrypt -lm -Wl,--no-whole-archive

# At this level, we don't know if anything in oaldir has changed, so always go
# down into that dir and do a make.
force_oal:
	mkdir -p $(OALDIR)
	make -C $(OALDIR) -f $(mdir)/$(MODDIR)/$(OALDIR)/Makefile


# Set VPATH because we want to compile in a seperate dir than source.
name := $(lastword $(MAKEFILE_LIST))
mdir := $(realpath $(dir $(name)))
VPATH=$(mdir)

# Generate and use dependencies.
CFLAGS += -MD
-include $(OBJS:%.o=%.d)

CFLAGS += -DBRCM_CHIP_HEX=0x$(BRCM_CHIP)
