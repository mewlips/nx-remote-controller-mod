#!/bin/sh

source /opt/usr/apps/nx-remote-controller-mod/common.sh

windowid=$($XDOTOOL search --class di-camera-app)

xev_nx_pid="$(ps ax | grep "xev-nx -.. -bi" | grep -v grep | awk '{print $1}')"
if [ "$xev_nx_pid" != "" ]; then
    kill $xev_nx_pid
fi

if [ "$1" == "on" ]; then
    $XDOTOOL windowmap $windowid
elif [ "$1" == "off" ]; then
    $XEV_NX -rv -bi $windowid &> /dev/null &
    $XDOTOOL windowmap $windowid
elif [ "$1" == "video" ]; then
    $XEV_NX -tr -bi $windowid &> /dev/null &
    $XDOTOOL windowunmap $windowid
elif [ "$1" == "osd" ]; then
    echo #TODO
fi
