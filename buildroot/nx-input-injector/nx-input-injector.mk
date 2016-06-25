################################################################################
#
# nx-input-injector
#
################################################################################

NX_INPUT_INJECTOR_VERSION = 1.0.0
NX_INPUT_INJECTOR_SITE = $(TOPDIR)/package/nx-input-injector/src
NX_INPUT_INJECTOR_SITE_METHOD = local
NX_INPUT_INJECTOR_LICENSE = GPLv3
NX_INPUT_INJECTOR_LICENSE_FILES = LICENSE
NX_INPUT_INJECTOR_DEPENDENCIES = xdotool-nx

define NX_INPUT_INJECTOR_BUILD_CMDS
	$(TARGET_CC) -o $(@D)/nx-input-injector $(@D)/nx-input-injector.c -lxdo
endef

define NX_INPUT_INJECTOR_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 755 $(@D)/nx-input-injector $(TARGET_DIR)/usr/bin/
endef

$(eval $(generic-package))
