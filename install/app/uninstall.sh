#!/bin/bash

source /opt/usr/apps/nx-remote-controller-mod/common.sh

confirm_dialog() {
    $YAD --text="<big>\n Do you really want to uninstall?\n</big>" \
         --button=gtk-no:1 --button=gtk-yes:0 --buttons-layout=spread \
         --center --width=650
}

if confirm_dialog; then
    $APP_PATH/off_nx-remote-controller-daemon.sh
    if [ -f /opt/usr/nx-on-wake/auto/nx-remote-controller-daemon.sh ]; then
        rm -f /opt/usr/nx-on-wake/auto/nx-remote-controller-daemon.sh
    fi
    if [ -f /opt/home/scripts/auto/nx-remote-controller-daemon.sh ]; then
        rm -f /opt/home/scripts/auto/nx-remote-controller-daemon.sh
    fi
    rm -rf $APP_PATH
    sync; sync; sync
fi
