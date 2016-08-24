#!/bin/bash

APP_PATH=/opt/usr/apps/nx-remote-controller-mod

xmodmap -e "keycode 223 = Help"

killall nx-input-injector
killall xev-nx
killall nx-remote-controller-daemon

$APP_PATH/lcd_control.sh on
if [ "$1" == "debug" ]; then
    nice --adjustment=19 $APP_PATH/nx-remote-controller-daemon
else
    nice --adjustment=19 $APP_PATH/nx-remote-controller-daemon &> /dev/null &
fi

exit 0;
