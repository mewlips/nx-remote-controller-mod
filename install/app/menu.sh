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

res1=$(mktemp -t nx-remote.XXXXXXXX)
res2=$(mktemp -t nx-remote.XXXXXXXX)

KEY="12345"
IP_ADDRESS=$(ifconfig $NET_DEVICE | grep inet | sed -e 's/.*addr://' -e 's/ .*//')
CONNECTED_AP=$(iwconfig $NET_DEVICE | grep ESSID | sed -e 's/.*://' -e 's/\"//g' -e 's/ .*//g')

execute_from_fifo() {
    local cmd
    while true; do
        cmd=$(cat $TOOLS_PATH/fifo)
        echo run cmd = $cmd
        $cmd
    done
}

main_menu() {
    ipcrm -M "$KEY"

    if [ ! -e $TOOLS_PATH/fifo ]; then
        $CHROOT mkfifo /fifo
    fi

    $YAD --plug=$KEY --tabnum=1 \
        --separator='\n' --quoted-output \
        --form --field='IP Address:' "$IP_ADDRESS" \
        --form --field='Connected AP:' "$CONNECTED_AP" \
        --form --field='Open Wi-Fi Settings:FBTN' "sh -c \"echo $APP_PATH/wifi.sh > /fifo\"" \
        --form --field='Open Original Mobile App:FBTN' "sh -c \"echo $APP_PATH/mobile.sh > /fifo\"" &

    $YAD --plug=$KEY --tabnum=2 \
         --separator='\n' --quoted-output \
         --text='Camera Hacks' --text-align=center \
         --form --field="Shutter::cb" "$($APP_PATH/shutter.sh get)" \
         --form --field="LCD::cb" "On!Off!Video" > $res1 &

    $YAD --plug=$KEY --tabnum=3 \
         --text="$ABOUT_TEXT" > $res2 &

    settings=$($YAD --undecorated --notebook --key=$KEY --tab="Main" --tab="Hacks" --tab="About")
}

killall yad
execute_from_fifo &
main_menu
result=$?

#eval TAB1="$(< $res1)"
#eval TAB2="$(< $res2)"

#echo ${TAB1[@]}
cat $res1
cat $res2

rm -f $res1 $res2

case $result in
    0) # OK
        ;;
    1) # Cancel
        ;;
    *)
        ;;
esac

killall yad
killall menu.sh
