#!/bin/bash

APP_PATH=/opt/usr/apps/nx-remote-controller-mod
TOOLS_PATH=$APP_PATH/tools
CHROOT="chroot $TOOLS_PATH"
YAD="$CHROOT yad"

ABOUT_TEXT="\
<b><span color='blue'>NX Remote Controller Mod (ver. 0.8)</span></b>
============================================
<i>Project Homepage:</i>
<small><span color='blue'>https://mewlips.github.io/nx-remote-controller-mod</span></small>

<i>GitHub:</i>
<small><span color='blue'>https://github.com/mewlips/nx-remote-controller-mod</span></small>

<i>Copyright:</i> Mewlips &lt;mewlips@gmail.com&gt;

<i>License:</i> GPL-3"

$YAD --text="$ABOUT_TEXT" \
     --button="Uninstall:0" \
     --button=gtk-ok:0 \
     --buttons-layout=edge

