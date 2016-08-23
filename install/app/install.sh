#!/bin/bash

ON_WAKE=/opt/storage/sdcard/nx-on-wake/on-wake
INSTALL_PATH=/opt/storage/sdcard/app
APP_PATH=/opt/usr/apps/nx-remote-controller-mod
TOOLS_PATH=$APP_PATH/tools
EXT_APP_PATH=/tmp/externals
NX_MODEL=$EXT_APP_PATH/nx-model
POPUP_TIMEOUT=$EXT_APP_PATH/popup_timeout
POPUP_OK=$EXT_APP_PATH/popup_ok

mkdir -p $EXT_APP_PATH
cp -av $INSTALL_PATH/externals/{nx-model,popup_ok,popup_timeout} $EXT_APP_PATH

# ========= force LCD on (crash on EVF) =========
st app disp lcd
sleep 1

# ========= only run on suitable firmware =========
if [ "$($NX_MODEL)" != nx500 ] && [ "$($NX_MODEL)" != nx1 ]; then
    $POPUP_OK "Camera or firmware not supported.\n\nPress OK to exit"
    exit
fi

# ========= some useful functions =========
show_msg() {
    killall popup_timeout
    $POPUP_TIMEOUT "$1" 100 &
}

show_msg " [ nx-remote-controller-mod ] "
sleep 2

# ========= clean-up existing mod ========
show_msg " [ Removing old version... ] "
if [ -d $APP_PATH ]; then
    sleep 2
    rm -rfv $APP_PATH
fi

# ========= copy stuff ========
show_msg " [ Installing files ... ] "
mkdir -pv $TOOLS_PATH
tar -C $TOOLS_PATH -xf $INSTALL_PATH/tools.tar
mkdir -p $TOOLS_PATH/{sbin,/usr/sbin}
chown root:root $TOOLS_PATH/bin/busybox
chroot $TOOLS_PATH /bin/busybox --install -s
mknode $TOOLS_PATH/dev/null c 1 3
rm -fv $INSTALL_PATH/tools.tar
cp -rv $INSTALL_PATH/* $APP_PATH
mv $APP_PATH/EV_MOBILE.sh /opt/usr/nx-on-wake/

# ========= clean up ========
rm -rfv $INSTALL_PATH
rm -fv $APP_PATH/install.sh
rm -fv $ON_WAKE
rm -rfv $EXT_APP_PATH

sync; sync; sync;
killall popup_timeout
$APP_PATH/externals/popup_ok "[ Installation completed! ]<br>1. Make sure the camera and the android device are on the Wi-Fi network.<br>2. Press the EV + MOBILE buttons to run the app launcher." "" "NEXT"
$APP_PATH/externals/popup_ok "3. Select 'Remote Controller' and 'Start'.<br>4. Launch the Android NX Remote Controller App." "" "OK"
