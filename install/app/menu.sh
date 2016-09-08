#!/bin/bash

APP_PATH=/opt/usr/apps/nx-remote-controller-mod
TOOLS_PATH=$APP_PATH/tools
CHROOT="chroot $TOOLS_PATH"
YAD="$CHROOT yad"

# turn on lcd
$APP_PATH/lcd_control.sh on

#TODO: keep alive camera
NX_MODEL=$(cat /etc/version.info | head -n 2 | tail -n 1)
echo $NX_MODEL

is_nx1_nx500() {
    [ "$NX_MODEL" == "NX500" ] || [ "$NX_MODEL" == "NX1" ] 
}

is_nx300() {
    if is_nx1_nx500; then
        false
    else
        true
    fi
}

if is_nx1_nx500; then
    #OPT_GEOMETRY="--geometry=720x480+0+0"
    OPT_GEOMETRY="--geometry=720x480"
    NET_DEVICE="mlan0"
else
    OPT_GEOMETRY="--geometry=800x480+0+0"
    NET_DEVICE="wlan0"
fi

TITLE="<b><span fgcolor='yellow' bgcolor='#1010ff'>\
       NX Remote Controller Mod (v0.8)       </span></b>"

howto() {
    echo "\
Connect your PC or mobile device to the
  same Wi-Fi network with this camera.
Open http://$IP_ADDRESS in web browser."
}

get_network_info() {
    IP_ADDRESS=$(ifconfig $NET_DEVICE | grep inet | sed -e 's/.*addr://' -e 's/ .*//')
    CONNECTED_AP=$(iwconfig $NET_DEVICE | grep ESSID | sed -e 's/.*://' -e 's/\"//g' -e 's/ .*//g')
}

run_wifi_settings() {
    $APP_PATH/externals/poker \
        /tmp/var/run/memory/ap_setting/request_type 0x0:2900000001000000
    sleep 2
    while true; do
        if [ "ap-setting-app" != "$($CHROOT xdotool getactivewindow getwindowname)" ]; then
            break;
        fi
        echo ap-setting-app
        sleep 1
    done
}

main_menu() {
    SPAN="<span color='blue'><i>"
    CSPAN="</i></span>"
    settings=$($YAD \
        --undecorated --scroll \
        --form --field="${SPAN}IP Address${CSPAN}:RO" "$IP_ADDRESS" \
        --form --field="${SPAN}Connected Wi-Fi AP${CSPAN}:RO" "$CONNECTED_AP" \
        --form --field=":LBL" "" \
        --form --field="${SPAN}Shutter Type${CSPAN}:cb" "$($APP_PATH/shutter.sh get)" \
        --form --field="${SPAN}LCD Control${CSPAN}:cb" "on!off!video" \
        --form --field=":LBL" "" \
        --form --field="Usage:TXT" "$(howto)" \
        --button="About:sh -c \"echo $APP_PATH/about.sh > /fifo\"" \
        --button="Mobile:sh -c \"echo $APP_PATH/mobile.sh > /fifo\"" \
        --button="WiFi:sh -c \"echo $APP_PATH/wifi.sh > /fifo\"" \
        --button=gtk-cancel:1 \
        --button=gtk-ok:0 \
        --text="$TITLE" \
        )
}

killall yad

get_network_info
if [ "$IP_ADDRESS" == "" ]; then
    run_wifi_settings
    for i in $(seq 10); do
        sleep 1
        get_network_info
        if [ "$IP_ADDRESS" != "" ]; then
            break
        fi
    done
fi
main_menu
result=$?

lcd_off_info() {
    $CHROOT yad --timeout=5 --timeout-indicator=left \
     --text="<big>To turn on the LCD again, \npush the 'MOBILE' button.</big>" \
     --button=gtk-ok --center --width=600
}

case $result in
    0) # OK
        shutter=$(echo "$settings" | cut -d'|' -f 4)
        lcd=$(echo "$settings" | cut -d'|' -f 5)
        $APP_PATH/shutter.sh $shutter
        if [ "$lcd" != "on" ]; then
            lcd_off_info
            $APP_PATH/lcd_control.sh $lcd
        fi
        ;;
    1) # Cancel
        ;;
    *)
        ;;
esac
