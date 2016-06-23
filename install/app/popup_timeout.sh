#!/bin/sh

source /opt/usr/apps/nx-remote-controller-mod/common.sh

timeout=$1
shift

args=""
for arg in "$@"; do
    args+=" $arg"
done
$POPUP_TIMEOUT "$args" $timeout
