#!/bin/sh

TARBALL=buildroot-2016.05.tar.bz2
DEFCONFIG=nx_remote_controller_mod_defconfig
BUILDROOT_DIR=buildroot-2016.05

tar_tools() {
    pushd output/target
    mkdir -p caps tmp
    tar --owner=root --group=root -chvf ../../../../../install/app/tools.tar \
        bin/busybox \
        caps/ \
        lib/ld-linux.so.3 \
        lib/libc.so.6 \
        lib/libdl.so.2 \
        lib/libgcc_s.so.1 \
        lib/libm.so.6 \
        lib/libpthread.so.0 \
        lib/librt.so.1 \
        tmp/ \
        usr/bin/convert \
        usr/bin/nx-input-injector \
        usr/bin/strace \
        usr/bin/xdotool \
        usr/bin/xev-nx \
        usr/bin/xwd \
        usr/bin/xwininfo \
        usr/lib/libMagickCore-6.Q16.so.2 \
        usr/lib/libMagickWand-6.Q16.so.2 \
        usr/lib/libX11.so.6 \
        usr/lib/libXau.so.6 \
        usr/lib/libXdmcp.so.6 \
        usr/lib/libXext.so.6 \
        usr/lib/libXi.so.6 \
        usr/lib/libXinerama.so.1 \
        usr/lib/libXrandr.so.2 \
        usr/lib/libXrender.so.1 \
        usr/lib/libXtst.so.6 \
        usr/lib/libjpeg.so.8 \
        usr/lib/libpng16.so.16 \
        usr/lib/libxcb-shape.so.0 \
        usr/lib/libxcb.so.1 \
        usr/lib/libxdo.so.3 \
        usr/lib/libxkbcommon.so.0 \
        usr/lib/libxkbfile.so.1 \
        usr/lib/libz.so.1
    popd
}

wget -c https://buildroot.org/downloads/$TARBALL

if [ -f $TARBALL ] && [ ! -d $BUILDROOT_DIR ]; then
    tar -xjf $TARBALL
    (cd $BUILDROOT_DIR/package; ln -s ../../../../xev-nx)
    (cd $BUILDROOT_DIR/package; ln -s ../../xdotool-nx)
    (cd $BUILDROOT_DIR/package; ln -s ../../../../nx-input-injector)
    (cd $BUILDROOT_DIR; patch -p1 < ../patch-001-libxdo-xev-nx.patch)
    (cd $BUILDROOT_DIR; patch -p1 < ../patch-002-nx-input-injector.patch)
fi

cp -fv nx_remote_controller_mod_defconfig $BUILDROOT_DIR/configs
cd $BUILDROOT_DIR
make nx_remote_controller_mod_defconfig && make && tar_tools && \
    mkdir -p ../../../sd_install/remote/tools && \
    tar -C ../../../sd_install/remote/tools -xf ../../../install/app/tools.tar
