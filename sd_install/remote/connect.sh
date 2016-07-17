#!/bin/sh
#
# Copyright 2014 Jonathan Dieter <jdieter@gmail.com>
# Distributed under the terms of the GNU General Public
# License v2 or later

if [ ! -e /mnt/mmc/remote/config ]; then
    echo "No config file at /mnt/mmc/remote/config"
    exit 1
fi

. /mnt/mmc/remote/config

iwconfig wlan0
if [ "$?" -eq "0" ]; then
    iwconfig wlan0 | grep -q "ESSID:\"$ESSID\""
    if [ "$?" -eq "0" ]; then # We're already connected to the right ESSID; successfully exit
        exit 0
    fi
    iwconfig wlan0 | grep -q "ESSID:off/any"
    if [ "$?" -ne "0" ]; then # We're connected to the wrong ESSID; exit with error
        iwconfig wlan0
        echo "Connected to wrong ESSID, exitting"
        exit 1
    fi
    /mnt/mmc/remote/disconnect.sh
fi

/usr/sbin/connmand -W nl80211 -r
/usr/sbin/net-config
dbus-send --system --dest=net.netconfig --print-reply /net/netconfig/wifi net.netconfig.wifi.LoadDriver string:"wifi"
sleep 2

for d in `ls /var/lib/connman`; do
    if [ ! -d "/var/lib/connman/$d" ]; then
        continue
    fi
    echo "Checking stored WiFi connection $d"

    grep -q "^Name=$ESSID$" /var/lib/connman/"$d"/settings
    if [ "$?" -eq "0" ]; then
        echo "Connecting to $ESSID using $d"
        dbus-send --system --print-reply --dest=net.connman "/net/connman/service/$d" net.connman.Service.Connect
        if [ "$?" -eq "0" ]; then #We've successfully connected
            sleep 2
            /mnt/mmc/remote/start.sh
        fi
    fi
done

echo "Unable to connect to WiFi"
exit 1
