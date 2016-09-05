#!/bin/sh

APP_PATH=/opt/usr/apps/nx-remote-controller-mod
TOOLS_PATH=$APP_PATH/tools
CHROOT="chroot $TOOLS_PATH"
YAD="$CHROOT yad"

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

ABOUT_TEXT="\
<small><small><b>NX Remote Controller Mod (ver. 0.8)</b>

<i>Project Homepage:</i>
<small><span color='blue'>https://mewlips.github.io/nx-remote-controller-mod</span></small>

<i>GitHub:</i>
<small><span color='blue'>https://github.com/mewlips/nx-remote-controller-mod</span></small>

<i>Copyright:</i> Mewlips &lt;mewlips@gmail.com&gt;

<i>License:</i> GPL-3</small></small>"

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
    settings=$($YAD \
        --undecorated --separator='\n' --quoted-output --scroll --columns=2 \
        --form --field='IP Addr:RO' "$IP_ADDRESS" \
        --form --field='WiFi AP:RO' "$CONNECTED_AP" \
        --form --field="Shutter:cb" "$($APP_PATH/shutter.sh get)" \
        --form --field="LCD:cb" "On!Off!Video" \
        --form --field='Wi-Fi Settings:FBTN' "sh -c \"echo $APP_PATH/wifi.sh > /fifo\"" \
        --form --field='Stock Mobile:FBTN' "sh -c \"echo $APP_PATH/mobile.sh > /fifo\"" \
        --form --field='Uninstall:FBTN' "sh -c \"echo $APP_PATH/uninstall.sh > /fifo\"" \
        --form --field='Telnetd:FBTN' "sh -c \"echo $APP_PATH/telnetd.sh > /fifo\"" \
        ) \
#         --text="$ABOUT_TEXT" > $res2 &
#    settings=$($YAD --undecorated --notebook --key=$KEY --tab="Main" --tab="Hacks" --tab="About")
}

killall yad

get_network_info
if [ "$IP_ADDRESS" == "" ]; then
    run_wifi_settings
    sleep 1
    get_network_info
fi
main_menu
result=$?

#eval TAB1="$(< $res1)"
#eval TAB2="$(< $res2)"

#echo ${TAB1[@]}
echo settings = $settings

case $result in
    0) # OK
        ;;
    1) # Cancel
        ;;
    *)
        ;;
esac

killall cat
killall yad
killall menu.sh
