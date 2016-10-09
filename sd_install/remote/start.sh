#!/bin/bash

killall nx-input-injector
killall xev-nx
killall nx-remote-controller-daemon

export APP_PATH=/mnt/mmc/remote

$APP_PATH/lcd_control.sh on
if [ "$1" == "debug" ]; then
    $APP_PATH/nx-remote-controller-daemon
else
    $APP_PATH/nx-remote-controller-daemon &> /dev/null &
fi

exit 0;
