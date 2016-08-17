#!/bin/sh

TARBALL=buildroot-2016.05.tar.bz2
DEFCONFIG=nx_remote_controller_mod_defconfig
BUILDROOT_DIR=buildroot-2016.05

tar_tools() {
    pushd output/target
    mkdir -p tmp
    cp -fv ../../../gtkrc-2.0 root/.gtkrc-2.0
    tar --owner=root --group=root -chvf ../../../../../install/app/tools.tar \
        bin/busybox \
        lib/ld-2.22.so \
        lib/ld-linux.so.3 \
        lib/libatomic.so.1 \
        lib/libc.so.6 \
        lib/libdl.so.2 \
        lib/libgcc_s.so.1 \
        lib/libm.so.6 \
        lib/libpthread.so.0 \
        lib/libresolv.so.2 \
        lib/librt.so.1 \
        root/.gtkrc-2.0 \
        tmp/ \
        usr/bin/convert \
        usr/bin/nx-input-injector \
        usr/bin/strace \
        usr/bin/xdotool \
        usr/bin/xev-nx \
        usr/bin/xvkbd \
        usr/bin/xwd \
        usr/bin/xwininfo \
        usr/bin/yad \
        usr/lib/libICE.so.6 \
        usr/lib/libMagickCore-6.Q16.so.2 \
        usr/lib/libMagickWand-6.Q16.so.2 \
        usr/lib/libSM.so.6 \
        usr/lib/libX11.so.6 \
        usr/lib/libXau.so.6 \
        usr/lib/libXaw.so.7 \
        usr/lib/libXdmcp.so.6 \
        usr/lib/libXext.so.6 \
        usr/lib/libXfixes.so.3 \
        usr/lib/libXi.so.6 \
        usr/lib/libXinerama.so.1 \
        usr/lib/libXmu.so.6 \
        usr/lib/libXpm.so.4 \
        usr/lib/libXrandr.so.2 \
        usr/lib/libXrender.so.1 \
        usr/lib/libXt.so.6 \
        usr/lib/libXtst.so.6 \
        usr/lib/libatk-1.0.so.0 \
        usr/lib/libcairo.so.2 \
        usr/lib/libexpat.so.1 \
        usr/lib/libffi.so.6 \
        usr/lib/libfontconfig.so.1 \
        usr/lib/libfreetype.so.6 \
        usr/lib/libgdk-x11-2.0.so.0 \
        usr/lib/libgdk_pixbuf-2.0.so.0 \
        usr/lib/libgio-2.0.so.0 \
        usr/lib/libglib-2.0.so.0 \
        usr/lib/libgmodule-2.0.so.0 \
        usr/lib/libgobject-2.0.so.0 \
        usr/lib/libgthread-2.0.so.0 \
        usr/lib/libgtk-x11-2.0.so.0 \
        usr/lib/libharfbuzz.so.0 \
        usr/lib/libjpeg.so.8 \
        usr/lib/libpango-1.0.so.0 \
        usr/lib/libpangocairo-1.0.so.0 \
        usr/lib/libpangoft2-1.0.so.0 \
        usr/lib/libpcre.so.1 \
        usr/lib/libpixman-1.so.0 \
        usr/lib/libpng16.so.16 \
        usr/lib/libxcb-render.so.0 \
        usr/lib/libxcb-shape.so.0 \
        usr/lib/libxcb-shm.so.0 \
        usr/lib/libxcb.so.1 \
        usr/lib/libxdo.so.3 \
        usr/lib/libxkbcommon.so.0 \
        usr/lib/libxkbfile.so.1 \
        usr/lib/libz.so.1 \
        usr/share/fonts/liberation/*

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
