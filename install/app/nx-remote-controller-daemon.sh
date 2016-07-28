#!/bin/bash

APP_PATH=/opt/usr/apps/nx-remote-controller-mod
EXT_APP_PATH=$APP_PATH/externals
POPUP_TIMEOUT=$EXT_APP_PATH/popup_timeout

killall nx-input-injector
killall xev-nx
killall nx-remote-controller-daemon

$APP_PATH/lcd_control.sh on
if [ "$1" == "debug" ]; then
    $APP_PATH/nx-remote-controller-daemon
else
    $APP_PATH/nx-remote-controller-daemon &> /dev/null &
fi

$POPUP_TIMEOUT " [ nx-remote-controller-mod ] " 2 &

exit 0;
