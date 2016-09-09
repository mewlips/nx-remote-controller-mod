#!/bin/bash

source /opt/usr/apps/nx-remote-controller-mod/common.sh

FIFO_PATH=$TOOLS_PATH/fifo

if [ ! -e $FIFO_PATH ]; then
    $CHROOT mkfifo /fifo
fi

di_camera_app_wid=$($XDOTOOL search --class di-camera-app)

mod_key() {
    while true; do
        if [ "$($XDOTOOL getactivewindow getwindowname)" == "YAD" ]; then
            $XDOTOOL key --window $di_camera_app_wid XF86AudioRaiseVolume # keep alive
            xmodmap -e "keycode 133 = Escape"
        else 
            xmodmap -e "keycode 133 = Super_L"
        fi
        sleep 1
    done
}

mod_key &

while true; do
    cmd=$(cat $FIFO_PATH)
    if is_nx500; then
        echo > $FIFO_PATH
    fi
    if [ "$cmd" != "" ]; then
        echo execute command : $cmd
        $cmd
    else
        sleep 1
    fi
done
