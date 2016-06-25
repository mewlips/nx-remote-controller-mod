################################################################################
#
# xdotool
#
################################################################################

XDOTOOL_NX_VERSION = v3.20150503.1
XDOTOOL_NX_SITE = $(call github,jordansissel,xdotool,$(XDOTOOL_NX_VERSION))
XDOTOOL_NX_LICENSE = BSD-3c
XDOTOOL_NX_LICENSE_FILES = COPYRIGHT
XDOTOOL_NX_DEPENDENCIES = xlib_libXtst xlib_libXinerama libxkbcommon xlib_libX11
XDOTOOL_NX_INSTALL_STAGING = YES

define XDOTOOL_NX_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(TARGET_CONFIGURE_OPTS) $(MAKE) -C $(@D)
endef

define XDOTOOL_NX_INSTALL_STAGING_CMDS
	$(TARGET_MAKE_ENV) $(TARGET_CONFIGURE_OPTS) $(MAKE) -C $(@D) \
		pre-install installlib installheader \
		PREFIX="$(STAGING_DIR)/usr"
endef

# Avoid 'install' target to skip 'post-install' which runs ldconfig on host
define XDOTOOL_NX_INSTALL_TARGET_CMDS
	$(TARGET_MAKE_ENV) $(TARGET_CONFIGURE_OPTS) $(MAKE) -C $(@D) \
		pre-install installlib installprog installheader \
		PREFIX="$(TARGET_DIR)/usr"
endef

$(eval $(generic-package))
