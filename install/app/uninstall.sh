#!/bin/bash

source /opt/usr/apps/nx-remote-controller-mod/common.sh

if $POPUP_OK "Do you really want to uninstall?" "YES" "NO"; then
    $APP_DIR/off_nx-remote-controller-daemon.sh
#    swapoff $APP_DIR/swapfile
    cp $POUP_TIMEOUT /tmp/
    rm -rf $APP_DIR
    sync; sync; sync
    /tmp/popup_timeout " [ Uninstall completed. ]" 2
    rm -f /tmp/popup_timeout
else
    return_menu
fi
