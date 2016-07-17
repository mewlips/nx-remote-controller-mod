#!/bin/sh
#
# Copyright 2014 Jonathan Dieter <jdieter@gmail.com>
# Distributed under the terms of the GNU General Public
# License v2 or later

dbus-send --system --dest=net.netconfig --print-reply /net/netconfig/wifi net.netconfig.wifi.RemoveDriver
dbus-send --system --dest=net.netconfig --print-reply /net/netconfig/wifi net.netconfig.wifi.ShutDownDriver
killall net-config
killall connmand

