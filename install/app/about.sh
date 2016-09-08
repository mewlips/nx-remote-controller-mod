#!/bin/bash

APP_PATH=/opt/usr/apps/nx-remote-controller-mod
TOOLS_PATH=$APP_PATH/tools
CHROOT="chroot $TOOLS_PATH"
YAD="$CHROOT yad"

TITLE="<b><span fgcolor='yellow' bgcolor='#1010ff'>\
       NX Remote Controller Mod (v0.8)      </span></b>"

ABOUT_TEXT="\
$TITLE

<i>Project Homepage:</i>
<small><span color='blue'>https://mewlips.github.io/nx-remote-controller-mod</span></small>

<i>GitHub:</i>
<small><span color='blue'>https://github.com/mewlips/nx-remote-controller-mod</span></small>

<i>Copyright:</i> Mewlips &lt;mewlips@gmail.com&gt;

<i>License:</i> GPL-3"

$YAD --text="$ABOUT_TEXT" \
     --form --field=":LBL" \
     --button="Uninstall:1" \
     --button="Kill daemon:2" \
     --button=gtk-ok:0 \
     --buttons-layout=edge

result=$?
if [ "$result" == "1" ]; then
    killall yad
    $APP_PATH/uninstall.sh
elif [ "$result" == "2" ]; then
    $APP_PATH/off_nx-remote-controller-daemon.sh
fi
