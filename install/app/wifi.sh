#!/bin/sh

APP_PATH=/opt/usr/apps/nx-remote-controller-mod
TOOLS_PATH=$APP_PATH/tools
CHROOT="chroot $TOOLS_PATH"

$APP_PATH/externals/poker \
    /tmp/var/run/memory/ap_setting/request_type 0x0:2900000001000000
killall yad menu.sh
sleep 2
while true; do
    if [ "ap-setting-app" != "$($CHROOT xdotool getactivewindow getwindowname)" ]; then
        break;
    fi
    echo ap-setting-app
    sleep 1
done
$APP_PATH/menu.sh &
