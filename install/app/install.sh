#!/bin/bash

INSTALL_PATH=/opt/storage/sdcard/app
APP_PATH=/opt/usr/apps/nx-remote-controller-mod
TOOLS_PATH=$APP_PATH/tools
NX_PATCH_PATH=/opt/usr/nx-on-wake
NX_KS_PATH=/opt/home/scripts

# ========= check model and fw version =========

MODEL=$(cat /etc/version.info | head -n 2 | tail -n 1)
FWVER=$(cat /etc/version.info | head -n 1)

if [ "$MODEL" == "NX500" ]; then
    if [ "$FWVER" != "1.12" ]; then
        echo "NX500 firmware version 1.12 is required."
        exit 1
    fi
elif [ "$MODEL" == "NX1" ]; then
    if [ "$FWVER" != "1.41" ]; then
        echo "NX1 firmware version 1.41 is required."
        exit 1
    fi
elif [ "$MODEL" != "NX300" ]; then
    echo "$MODEL is not supported."
    exit 1
fi

if [ -d $APP_PATH ]; then
    echo "=== Removing old version... ==="
    rm -rfv $APP_PATH || exit 1
fi

echo "=== Installing files... ==="
mkdir -pv $TOOLS_PATH || exit 1
tar -C $TOOLS_PATH -xvf $INSTALL_PATH/tools.tar || exit 1
mkdir -pv $TOOLS_PATH/{dev,sbin,usr/sbin} || exit 1
chown root:root $TOOLS_PATH/bin/busybox || exit 1
chroot $TOOLS_PATH /bin/busybox --install -s || exit 1
mknod $TOOLS_PATH/dev/null c 1 3 || exit 1
rm -fv $INSTALL_PATH/tools.tar || exit 1
cp -rv $INSTALL_PATH/* $APP_PATH || exit 1
if [ -d $NX_PATCH_PATH/auto ]; then
    ln -sfv $APP_PATH/nx-remote-controller-daemon.sh $NX_PATCH_PATH/auto/ || exit 1
elif [ -d $NX_KS_PATH/auto ]; then
    ln -sfv $APP_PATH/nx-remote-controller-daemon.sh $NX_KS_PATH/auto/ || exit 1
fi

echo "=== Cleaning up files... ==="
rm -rfv $INSTALL_PATH || exit 1
rm -fv $APP_PATH/install.sh || exit 1

sync; sync; sync;

exit 0
