###################################################################################
#
# hello
#
###################################################################################


HELLO_VERSION:=1.0.0
HELLO_SITE:=$(CURDIR)/work/hello
HELLO_SITE_METHOD:=local
HELLO_INSTALL_TARGET:=YES

define HELLO_BUILD_CMDS
	$(MAKE) CC="$(TARGET_CC)" LD="$(TARGET_LD)" -C $(@D) all
endef

define HELLO_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 0755 $(@D)/hello $(TARGET_DIR)/bin
endef

define HELLO_PERMISSIONS
	/bin/hello f 4755 0 0 - - - - -
endef

$(eval $(generic-package))
