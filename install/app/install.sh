#!/bin/bash

ON_WAKE=/opt/storage/sdcard/nx-on-wake/on-wake
INSTALL_PATH=/opt/storage/sdcard/app
APP_PATH=/opt/usr/apps/nx-remote-controller-mod
TOOLS_PATH=$APP_PATH/tools
NX_PATCH_PATH=/opt/usr/apps/nx-on-wake
YAD="chroot $INSTALL_PATH/tools yad"

eval $($INSTALL_PATH/model.sh)

# ========= force LCD on (crash on EVF) =========
st app disp lcd
sleep 1

# ========= check model and fw version =========
if [ "$MODEL" == "NX500" ]; then
    if [ "$FWVER" != "1.12" ]; then
        $YAD --text="NX500 firmware version <b>1.12</b> is required."
        exit
    fi
elif [ "$MODEL" == "NX1" ]; then
    if [ "$FWVER" != "1.41" ]; then
        $YAD --text="NX1 firmware version <b>1.41</b> is required."
        exit
    fi
elif [ "$MODEL" != "NX300" ]; then
    $YAD --text="$MODEL is not supported."
    exit
fi

if [ -d $APP_PATH ]; then
    echo "=== Removing old version... ==="
    rm -rfv $APP_PATH
fi

echo "=== Installing files... ==="
mkdir -pv $TOOLS_PATH
tar -C $TOOLS_PATH -xvf $INSTALL_PATH/tools.tar
mkdir -pv $TOOLS_PATH/{sbin,/usr/sbin}
chown root:root $TOOLS_PATH/bin/busybox
chroot $TOOLS_PATH /bin/busybox --install -s
mknod $TOOLS_PATH/dev/null c 1 3
rm -fv $INSTALL_PATH/tools.tar
cp -rv $INSTALL_PATH/* $APP_PATH
mv -fv $APP_PATH/EV_MOBILE.sh $NX_PATCH_PATH
ln -sv $APP_PATH/nx-remote-controller-daemon.sh $NX_PATCH_PATH/auto/

echo "=== Cleaning up files... ==="
rm -rfv $INSTALL_PATH
rm -fv $APP_PATH/install.sh
rm -fv $ON_WAKE

sync; sync; sync;
echo "=== Installation completed! ==="
echo "Usage:"
echo "  Press the 'Mobile' button to run the menu of 'NX Remote Controller Mod'."
echo "  Long press the 'Mobile' button to run original mobile function."
echo
echo "Press 'OK' Button to continue."
