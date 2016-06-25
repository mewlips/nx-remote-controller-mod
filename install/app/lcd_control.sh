#!/bin/sh

windowid=$(chroot /opt/usr/apps/nx-remote-controller-mod/tools xdotool search --class di-camera-app)

xev_nx_pid="$(ps ax | grep "xev-nx -.. -bi" | grep -v grep | awk '{print $1}')"
if [ "$xev_nx_pid" != "" ]; then
    kill $xev_nx_pid
fi

if [ "$1" == "on" ]; then
    chroot /opt/usr/apps/nx-remote-controller-mod/tools xdotool windowmap $windowid
elif [ "$1" == "off" ]; then
    chroot /opt/usr/apps/nx-remote-controller-mod/tools xev-nx -rv -bi $windowid &> /dev/null &
    chroot /opt/usr/apps/nx-remote-controller-mod/tools xdotool windowmap $windowid
elif [ "$1" == "video" ]; then
    chroot /opt/usr/apps/nx-remote-controller-mod/tools xev-nx -tr -bi $windowid &> /dev/null &
    chroot /opt/usr/apps/nx-remote-controller-mod/tools xdotool windowunmap $windowid
elif [ "$1" == "osd" ]; then
    echo #TODO
fi
