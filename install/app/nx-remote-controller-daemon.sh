#!/bin/bash

APP_PATH=/opt/usr/apps/nx-remote-controller-mod
EXT_APP_PATH=$APP_PATH/externals
POPUP_TIMEOUT=$EXT_APP_PATH/popup_timeout

killall nx-input-injector
killall xev-nx
killall nx-remote-controller-daemon

di_camera_app_id=$(chroot $APP_PATH/tools xdotool search --class di-camera-app)

$APP_PATH/lcd_control.sh on
if [ "$1" == "debug" ]; then
    $APP_PATH/nx-remote-controller-daemon $di_camera_app_id
else
    $APP_PATH/nx-remote-controller-daemon $di_camera_app_id &> /dev/null &
fi

$POPUP_TIMEOUT " [ nx-remote-controller-mod ] " 2 &

exit 0;
