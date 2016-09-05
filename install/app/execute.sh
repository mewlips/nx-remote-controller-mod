#!/bin/sh

APP_PATH=/opt/usr/apps/nx-remote-controller-mod
TOOLS_PATH=$APP_PATH/tools
FIFO_PATH=$TOOLS_PATH/fifo
CHROOT="chroot $TOOLS_PATH"

if [ ! -e $FIFO_PATH ]; then
    $CHROOT mkfifo /fifo
fi

while true; do
    cmd=$(cat $FIFO_PATH)
    echo > $FIFO_PATH
    if [ "$cmd" != "" ]; then
        echo execute command : $cmd
        $cmd
    else
        sleep 1
    fi
done

