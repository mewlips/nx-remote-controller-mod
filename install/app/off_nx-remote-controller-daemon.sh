#!/bin/bash

killall nx-input-injector
killall xev-nx
killall nx-remote-controller-daemon

xmodmap -e "keycode 223 = XF86Mail"

exit 0
