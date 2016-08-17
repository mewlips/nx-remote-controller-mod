################################################################################
#
# xapp_xev
#
################################################################################

XEV_NX_VERSION = 1.2.2
XEV_NX_SITE = $(TOPDIR)/package/xev-nx/src
XEV_NX_SITE_METHOD = local
XEV_NX_LICENSE = MIT
XEV_NX_LICENSE_FILES = COPYING
XEV_NX_DEPENDENCIES = xlib_libX11 xlib_libXrandr

$(eval $(autotools-package))
