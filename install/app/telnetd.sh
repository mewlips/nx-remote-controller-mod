#!/bin/sh

source /opt/usr/apps/nx-remote-controller-mod/common.sh

killall telnetd
$APP_PATH/telnetd &
