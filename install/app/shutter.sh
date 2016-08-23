#!/bin/sh

mode="$1"

if [ "$mode" == "get" ]; then
    val=$(/usr/bin/st cap capdtm getusr ADJUSTSHUTTERTYPE | sed -e 's/.*(//' -e 's/).*//')
    if [ "$val" == "0x750001" ]; then
        echo '^Silent!Normal'
    else
        echo 'Silent!^Normal'
    fi
elif [ "$mode" == "Silent" ]; then
    /usr/bin/st cap capdtm setusr ADJUSTSHUTTERTYPE 0x750001
else
    /usr/bin/st cap capdtm setusr ADJUSTSHUTTERTYPE 0x750000
fi
