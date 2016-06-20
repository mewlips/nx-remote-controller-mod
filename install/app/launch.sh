#!/bin/bash

APP_DIR=/opt/usr/apps/nx-remote-controller-mod
TOOLS_DIR=$APP_DIR/tools

SWAP_FILE=$APP_DIR/swapfile
if [[ ! -f $SWAP_FILE ]]; then
    $APP_DIR/externals/popup_timeout  " [ Initializing swap, please wait... ] " 20 &
    dd if=/dev/zero of=$SWAP_FILE bs=1024 count=131072 && mkswap $SWAP_FILE
fi
swapon $SWAP_FILE
sysctl vm.swappiness=10
killall popup_timeout

killall nx-remote-controlller-daemon
$APP_DIR/nx-remote-controller-daemon &> /dev/null &
$APP_DIR/popup_timeout  " [ nx-remote-controller-mod ] " 2 &

exit 0;
