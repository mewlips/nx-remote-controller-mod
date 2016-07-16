#!/bin/sh

TOOLS_PATH=/opt/storage/sdcard/remote/tools
windowid=$(chroot $TOOLS_PATH xdotool search --class di-camera-app)

xev_nx_pid="$(ps ax | grep "xev-nx -.. -bi" | grep -v grep | awk '{print $1}')"
if [ "$xev_nx_pid" != "" ]; then
    kill $xev_nx_pid
fi

if [ "$1" == "on" ]; then
    chroot $TOOLS_PATH xdotool windowmap $windowid
elif [ "$1" == "off" ]; then
    chroot $TOOLS_PATH xev-nx -rv -bi $windowid &> /dev/null &
    chroot $TOOLS_PATH xdotool windowmap $windowid
elif [ "$1" == "video" ]; then
    chroot $TOOLS_PATH xev-nx -tr -bi $windowid &> /dev/null &
    chroot $TOOLS_PATH xdotool windowunmap $windowid
elif [ "$1" == "osd" ]; then
    echo #TODO
fi
