#!/bin/bash

killall nx-input-injector
killall xev-nx
killall nx-remote-controller-daemon
killall execute.sh
killall yad

xmodmap -e "keycode 223 = XF86Mail"
xmodmap -e "keycode 133 = Super_L"

exit 0
