#!/bin/sh
/opt/usr/apps/nx-remote-controller-mod/externals/poker \
    /tmp/var/run/memory/ap_setting/request_type 0x0:2900000001000000
killall yad menu.sh
sleep 2
while true; do
    if [ "ap-setting-app" != "$(chroot tools xdotool getactivewindow getwindowname)" ]; then
        break;
    fi
    echo ap-setting-app
    sleep 1
done
./menu.sh &
