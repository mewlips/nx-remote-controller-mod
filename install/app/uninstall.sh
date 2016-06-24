#!/bin/bash

source /opt/usr/apps/nx-remote-controller-mod/common.sh

if $POPUP_OK "Do you really want to uninstall?" "YES" "NO"; then
    $APP_PATH/off_nx-remote-controller-daemon.sh
#    swapoff $APP_PATH/swapfile
    cp $POPUP_TIMEOUT /tmp/
    rm -rf $APP_PATH
    sync; sync; sync
    /tmp/popup_timeout " [ Uninstall completed. ]" 2
    rm -f /tmp/popup_timeout
else
    return_menu
fi
