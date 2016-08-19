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

KEY="1123"

main_menu() {

    chroot tools yad --plug=$KEY --tabnum=1 \
         --text='Remote Controller' --text-align=center \
         --text-info --text=abc \
         --button=Original-Mobile-Menu:$ID_MOBILE \
         --button=Close:$ID_CLOSE --buttons-layout=edge &

    chroot tools yad --plug=$KEY --tabnum=2 \
         --text='Mod Menu' --text-align=center \
         --text-info --text=abc \
         --button=Original-Mobile-Menu:$ID_MOBILE \
         --button=Close:$ID_CLOSE --buttons-layout=edge &

    $YAD --notebook --key=$KEY --tab=Main --tab=About
}

main_menu
result=$?

case $result in
    $ID_CLOSE) # Close
        ;;
    $ID_MOBILE)
        echo "TODO: launch original mobile func"
        ;;
    *)
        ;;
esac
