#!/bin/bash

export APP_PATH=/opt/usr/apps/nx-remote-controller-mod
export HOME=/root
export USER=root

xmodmap -e "keycode 223 = Help"

killall nx-input-injector
killall xev-nx
killall nx-remote-controller-daemon
killall execute.sh

$APP_PATH/lcd_control.sh on
$APP_PATH/execute.sh &

if [ "$1" == "debug" ]; then
    nice --adjustment=19 $APP_PATH/nx-remote-controller-daemon
else
    nice --adjustment=19 $APP_PATH/nx-remote-controller-daemon &> /dev/null &
    for i in $(seq 1 3 7); do
        echo 1 > /sys/devices/platform/leds-gpio/leds/card/brightness
        sleep 0.$i
        echo 0 > /sys/devices/platform/leds-gpio/leds/card/brightness
        sleep 0.$i
    done
fi
