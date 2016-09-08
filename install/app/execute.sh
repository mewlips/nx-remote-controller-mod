#!/bin/bash

APP_PATH=/opt/usr/apps/nx-remote-controller-mod
TOOLS_PATH=$APP_PATH/tools
FIFO_PATH=$TOOLS_PATH/fifo
CHROOT="chroot $TOOLS_PATH"

if [ ! -e $FIFO_PATH ]; then
    $CHROOT mkfifo /fifo
fi

di_camera_app_wid=$($CHROOT xdotool search --class di-camera-app)

while true; do
    cmd=$(cat $FIFO_PATH)
    echo > $FIFO_PATH
    if [ "$cmd" != "" ]; then
        echo execute command : $cmd
        $cmd
    else
        if [ "$($CHROOT xdotool getactivewindow getwindowname)" == "YAD" ]; then
            $CHROOT xdotool key --window $di_camera_app_wid XF86AudioRaiseVolume # keep alive 
            xmodmap -e "keycode 133 = Escape"
        else 
            xmodmap -e "keycode 133 = Super_L"
        fi
        sleep 1
    fi
done
