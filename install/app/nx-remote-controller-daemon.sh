#!/bin/bash

APP_PATH=/opt/usr/apps/nx-remote-controller-mod
EXT_APP_PATH=$APP_PATH/externals
TOOLS_PATH=$APP_PATH/tools
POPUP_TIMEOUT=$EXT_APP_PATH/popup_timeout

killall nx-input-injector
killall xev-nx
killall nx-remote-controller-daemon
sleep 1

$APP_PATH/lcd_control.sh on
$APP_PATH/nx-remote-controller-daemon &> /dev/null &

$POPUP_TIMEOUT " [ nx-remote-controller-mod ] " 2 &

exit 0;
