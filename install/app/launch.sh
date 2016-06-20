#!/bin/bash

APP_DIR=/opt/usr/apps/nx-remote-controller-mod
EXT_APP_PATH=$APP_DIR/externals
TOOLS_DIR=$APP_DIR/tools
POPUP_TIMEOUT=$EXT_APP_PATH/popup_timeout

SWAP_FILE=$APP_DIR/swapfile
if [[ ! -f $SWAP_FILE ]]; then
    $POPUP_TIMEOUT " [ Initializing swap, please wait... ] " 20 &
    dd if=/dev/zero of=$SWAP_FILE bs=1024 count=131072 && mkswap $SWAP_FILE
    killall popup_timeout
fi
if free | grep Swap | [ $(awk '{ print $2 }') == 0 ]; then
    swapon $SWAP_FILE
    sysctl vm.swappiness=10
fi

killall nx-remote-controller-daemon
sleep 1

run_command_bg() {
    read cmd
    echo $cmd
    $cmd &
    run_command_bg
}

($APP_DIR/nx-remote-controller-daemon | run_command_bg) &> /dev/null &

$POPUP_TIMEOUT " [ nx-remote-controller-mod ] " 2 &

exit 0;
