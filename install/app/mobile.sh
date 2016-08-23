#!/bin/sh
di_camera_app_wid=$(chroot tools xdotool search di-camera-app)
chroot tools xdotool key --window $di_camera_app_wid XF86Mail
killall yad
