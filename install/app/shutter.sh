#!/bin/sh

source /opt/usr/apps/nx-remote-controller-mod/common.sh

mode="$1"

if [ "$mode" == "get" ]; then
    if is_nx1; then
        echo 'Not supported.'
        exit 0
    fi
    val=$(/usr/bin/st cap capdtm getusr ADJUSTSHUTTERTYPE | sed -e 's/.*(//' -e 's/).*//')
    if [ "$val" == "0x750001" ]; then
        echo '^Silent!Normal'
    else
        echo 'Silent!^Normal'
    fi
elif [ "$mode" == "Silent" ]; then
    /usr/bin/st cap capdtm setusr ADJUSTSHUTTERTYPE 0x750001
elif [ "$mode" == "Normal" ]; then
    /usr/bin/st cap capdtm setusr ADJUSTSHUTTERTYPE 0x750000
fi
