#!/bin/bash

killall -9 dfmsd
killall nx-input-injector
killall xev-nx
killall nx-remote-controller-daemon

APP_PATH=/opt/storage/sdcard/remote/

$APP_PATH/lcd_control.sh on
if [ "$1" == "debug" ]; then
    $APP_PATH/nx-remote-controller-daemon
else
    $APP_PATH/nx-remote-controller-daemon &> /dev/null &
fi

exit 0;
