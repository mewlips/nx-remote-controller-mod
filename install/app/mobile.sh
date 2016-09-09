#!/bin/sh

source /opt/usr/apps/nx-remote-controller-mod/common.sh

di_camera_app_wid=$($XDOTOOL search di-camera-app)
$XDOTOOL key --window $di_camera_app_wid XF86Mail

killall yad
