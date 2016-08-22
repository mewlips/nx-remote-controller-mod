#!/bin/sh

NX_MODEL=$(externals/nx-model)

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
else
    OPT_GEOMETRY="--geometry=800x480+0+0"
fi

YAD="chroot tools yad $OPT_GEOMETRY --undecorated"

ID_MOBILE=1
ID_CLOSE=2

KEY="12345"

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

main_menu() {
    ipcrm -M "$KEY"

    chroot tools yad --plug=$KEY --tabnum=1 \
         --separator='\n' --quoted-output \
         --text='Camera Hacks' --text-align=center \
         --form --field="Shutter::cb" "Silent!^Normal" \
         --form --field="LCD::cb" "On!Off!Video" > $res1 &

    chroot tools yad --plug=$KEY --tabnum=2 \
         --text="$ABOUT_TEXT" > $res2 &

    settings=$($YAD --notebook --key=$KEY --tab="Hacks" --tab=About \
                    --button="Orig. Mobile:$ID_MOBILE" \
                    --button=Close:"$ID_CLOSE" --buttons-layout=end)
}

main_menu
result=$?

eval TAB1=($(< $res1))
eval TAB2=($(< $res2))

echo ${TAB1[@]}

case $result in
    $ID_CLOSE) # Close
        ;;
    $ID_MOBILE)
        di_camera_app_wid=$(chroot tools xdotool search di-camera-app)
        chroot tools xdotool key --window $di_camera_app_wid XF86Mail
        #echo "TODO: launch original mobile func"
        ;;
    *)
        ;;
esac
