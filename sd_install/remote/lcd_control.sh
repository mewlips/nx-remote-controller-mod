#!/bin/sh

if [ "$1" == "on" ]; then
    st lcd set 3
elif [ "$1" == "off" ]; then
    st lcd set 4
fi
