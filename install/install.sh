#!/bin/bash

INSTALL_PATH=/opt/storage/sdcard/app
INSTALL_LOG=nx-remote-controller-mod-install.log
INSTALL_LOG_PATH=/opt/storage/sdcard/$INSTALL_LOG
TOOLS_PATH=/opt/usr/apps/nx-remote-controller-mod/tools

blink() {
    while [ -f /opt/storage/sdcard/install.sh ]; do
        echo 1 > /sys/devices/platform/leds-gpio/leds/card/brightness
        sleep 0.5
        echo 0 > /sys/devices/platform/leds-gpio/leds/card/brightness
        sleep 0.5
    done
}

blink &

# ========= force LCD on (crash on EVF) =========
st app disp lcd
sleep 1

$INSTALL_PATH/install.sh &> $INSTALL_LOG_PATH
result=$?

if [ -f /opt/storage/sdcard/install.sh ]; then
    rm -f /opt/storage/sdcard/install.sh
fi
if [ -f /opt/storage/sdcard/info.tg ]; then
    rm -f /opt/storage/sdcard/info.tg
fi
if [ -f /opt/storage/sdcard/nx_cs.adj ]; then
    rm -f /opt/storage/sdcard/nx_cs.adj
fi

killall dfmsd

sync;sync;sync;

if [ "$result" != 0 ]; then
    reboot
fi

INSTALL_COMPLETED_MSG="\
<big><span color='blue'>Installation completed!</span></big>

Usage:

  Press the 'MOBILE' button to run
    the menu of 'NX Remote Controller Mod'.

  Press 'OK' Button to reboot."

export HOME=/root
export USER=root
chroot $TOOLS_PATH yad \
    --text="$INSTALL_COMPLETED_MSG" \
    --button=gtk-ok:0 --buttons-layout=center

reboot
