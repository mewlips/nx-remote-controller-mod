#!/bin/sh

APP_PATH=/opt/usr/apps/nx-remote-controller-mod
TOOLS_PATH=$APP_PATH/tools
CHROOT="chroot $TOOLS_PATH"

di_camera_app_wid=$($CHROOT xdotool search di-camera-app)
$CHROOT xdotool key --window $di_camera_app_wid XF86Mail

killall yad
