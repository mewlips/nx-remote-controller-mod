#!/bin/sh

TARBALL=buildroot-2016.05.tar.bz2
DEFCONFIG=nx_remote_controller_mod_defconfig
BUILDROOT_DIR=buildroot-2016.05

tar_tools() {
    pushd output/target
    tar -chvf ../../../../install/app/tools.tar \
        bin/busybox  \
        lib/ld-linux.so.3 \
        lib/libc.so.6 \
        lib/libdl.so.2 \
        lib/libm.so.6 \
        lib/libpthread.so.0 \
        lib/librt.so.1 \
        usr/bin/strace \
        usr/bin/xdotool  \
        usr/bin/xev  \
        usr/lib/libX11.so.6 \
        usr/lib/libXau.so.6 \
        usr/lib/libXdmcp.so.6 \
        usr/lib/libXext.so.6 \
        usr/lib/libXi.so.6 \
        usr/lib/libXinerama.so.1 \
        usr/lib/libXrandr.so.2 \
        usr/lib/libXrender.so.1 \
        usr/lib/libXtst.so.6 \
        usr/lib/libxcb.so.1 \
        usr/lib/libxdo.so.3 \
        usr/lib/libxkbcommon.so.0
    popd
}

wget -c https://buildroot.org/downloads/$TARBALL

if [ -f $TARBALL ] && [ ! -d $BUILDROOT_DIR ]; then
    tar -xjf $TARBALL
fi

cp -fv nx_remote_controller_mod_defconfig $BUILDROOT_DIR/configs
cd $BUILDROOT_DIR
make nx_remote_controller_mod_defconfig && make && tar_tools
