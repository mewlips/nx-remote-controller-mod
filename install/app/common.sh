#!/bin/bash

APP_PATH=/opt/usr/apps/nx-remote-controller-mod
TOOLS_PATH=$APP_PATH/tools
CHROOT="chroot $TOOLS_PATH"
YAD="$CHROOT yad"
XDOTOOL="$CHROOT xdotool"
XEV_NX="$CHROOT xev-nx"

NX_MODEL=$(cat /etc/version.info | head -n 2 | tail -n 1)
is_nx1() {
    [ "$NX_MODEL" == "NX1" ] 
}
is_nx500() {
    [ "$NX_MODEL" == "NX500" ]
}
is_nx1_nx500() {
    is_nx1 || is_nx500
}
is_nx300() {
    if is_nx1_nx500; then
        false
    else
        true
    fi
}

TITLE="<b><span fgcolor='yellow' bgcolor='#1010ff'>\
       NX Remote Controller Mod (v0.9)      </span></b>"

