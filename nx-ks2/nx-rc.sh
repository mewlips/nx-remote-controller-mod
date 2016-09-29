#!/bin/bash

killall nx-input-injector
killall xev-nx
killall nx-remote-controller-daemon

export APP_PATH=/opt/usr/nx-ks/nx-rc
nice --adjustment=19 $APP_PATH/nx-remote-controller-daemon &> /dev/null &
