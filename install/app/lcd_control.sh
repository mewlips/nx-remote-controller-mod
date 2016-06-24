#!/bin/sh

windowid=$(chroot /opt/usr/apps/nx-remote-controller-mod/tools xwininfo -root -tree | grep --text di-camera-app | awk '{print $1}')

echo $windowid

if [ "$1" == "on" ]; then
    chroot /opt/usr/apps/nx-remote-controller-mod/tools xdotool windowmap $windowid
    #TODO: windowunmap black screen program
elif [ "$1" == "off" ]; then
    chroot /opt/usr/apps/nx-remote-controller-mod/tools xdotool windowunmap $windowid
    #TODO: windowmap black screen program
fi
