#!/bin/bash

APP_PATH=/opt/usr/apps/nx-remote-controller-mod
TOOLS_PATH=$APP_PATH/tools
CHROOT="chroot $TOOLS_PATH"
YAD="$CHROOT yad"

# turn on lcd
$APP_PATH/lcd_control.sh on

#TODO: keep alive camera
#TODO: parse /etc/version?? directly
NX_MODEL=$($APP_PATH/externals/nx-model)

is_nx1_nx500() {
    [ "$NX_MODEL" = "nx500" ] || [ "$NX_MODEL" = "nx1" ] 
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

TITLE="<b><span color='blue'>NX Remote Controller Mod (ver. 0.8)</span></b>"

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
        if [ "ap-setting-app" != "$(chroot tools xdotool getactivewindow getwindowname)" ]; then
            break;
        fi
        echo ap-setting-app
        sleep 1
    done
}

main_menu() {
#        --undecorated --separator='\n' --quoted-output --scroll
    settings=$($YAD \
        --undecorated --scroll \
        --form --field='IP Address:RO' "$IP_ADDRESS" \
        --form --field='Connected Wi-Fi AP:RO' "$CONNECTED_AP" \
        --form --field=":LBL" "" \
        --form --field="Shutter Type:cb" "$($APP_PATH/shutter.sh get)" \
        --form --field="LCD Control:cb" "on!off!video" \
        --form --field="$HOWTO_0:LBL" "" \
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

case $result in
    0) # OK
        shutter=$(echo "$settings" | cut -d'|' -f 4)
        lcd=$(echo "$settings" | cut -d'|' -f 5)
        $APP_PATH/shutter.sh $shutter
        $APP_PATH/lcd_control.sh $lcd
        ;;
    1) # Cancel
        ;;
    *)
        ;;
esac

killall cat
killall yad
killall menu.sh
