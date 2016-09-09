#!/bin/sh

source /opt/usr/apps/nx-remote-controller-mod/common.sh

$APP_PATH/externals/poker \
    /tmp/var/run/memory/ap_setting/request_type 0x0:2900000001000000
killall yad menu.sh
sleep 2
while true; do
    if [ "ap-setting-app" != "$($XDOTOOL getactivewindow getwindowname)" ]; then
        break;
    fi
    echo ap-setting-app
    sleep 1
done
$APP_PATH/menu.sh &
