include $(PFM_ROOT_PATH)/make/Makefile.config

_TARGET_NAME := tr69c
_SRC_PATHS   := $(PFM_TR69C_PATH)/src
_SRC_PATHS   += $(PFM_TR69C_PATH)/custom/$(PFM_DATA_MODEL)
_BUILD_PATH  := $(PFM_TR69C_BUILD_PATH)
CFLAGS       += -I$(PFM_TR69C_PATH)/custom
CFLAGS       += -I$(PFM_TR69C_PATH)/src/webproto
CFLAGS       += -I$(PFM_TR69C_PATH)/src/SOAPParser
CFLAGS       += -I$(PFM_TR69C_PATH)/include
CFLAGS       += -I$(PFM_FWK_PATH)/include
CFLAGS       += -I$(PFM_FWK_PATH)/include/util
CFLAGS       += -I$(PFM_FWK_PATH)/include/xml
CFLAGS       += -I$(PFM_FWK_PATH)/include/vos
CFLAGS       += -I$(PFM_CMC_PATH)/src
CFLAGS       += -I$(PFM_CMC_PATH)/include
CFLAGS       += -I$(PFM_CMC_PATH)/src/phl
CFLAGS       += -I$(PFM_CMC_PATH)/src/lck
CFLAGS       += -I$(PFM_CMC_CORE_PATH)/include
CFLAGS       += -I$(PFM_CMC_API_PATH)/include
CFLAGS       += -I$(PFM_CMC_API_PATH)/include/mdm
CFLAGS       += -I$(PFM_CMC_EDM_PATH)/$(PFM_DATA_MODEL)/include
CFLAGS       += -I$(PFM_HAL_UTIL_PATH)/include
CFLAGS       += -I$(PFM_HAL_UTIL_PATH)/include/modules
CFLAGS       += -I$(PFM_HAL_API_PATH)/include/modules
CFLAGS       += -I$(PFM_OPENSSL_PATH)/include
CFLAGS       += -DDMP_X_CT_COM_USERINFO_1
CFLAGS       += -DDMP_X_CT_COM_1
CFLAGS       += -DDMP_X_BROADCOM_COM_DEBUG_1
CFLAGS       += -DDMP_X_BROADCOM_COM_SECURITY_1
CFLAGS       += -DDMP_X_CT_COM_REMOTE_STATUS_1

_LIBS        += $(PFM_FWK_BUILD_PATH)/libfwk.so
_LIBS        += $(PFM_CMC_API_BUILD_PATH)/libcmc_api.so
_LIBS        += $(PFM_HAL_UTIL_BUILD_PATH)/libhal_util.so
ifeq ($(strip $(PFM_SIM)),)
_LIBS        += $(PFM_PRODUCT_SOLUTION_PATH)/lib/libssl.so
_LIBS        += $(PFM_PRODUCT_SOLUTION_PATH)/lib/libcrypto.so
else
_LIBS        += $(PFM_OPENSSL_PATH)/openssl-0.9.7f/libssl.so
_LIBS        += $(PFM_OPENSSL_PATH)/openssl-0.9.7f/libcrypto.so
endif
_LIBS        += $(PFM_HAL_UTIL_DEP_LIBS)
_LIBS        += -lpthread
LDFLAGS      +=
_INSTALL_PATH += $(PFM_BIN_INSTALL_PATH)

.PHONY: prepare all install clean
prepare: 
all: _all_
install: _install_
clean: _clean_

include $(PFM_ROOT_PATH)/make/Makefile.user
