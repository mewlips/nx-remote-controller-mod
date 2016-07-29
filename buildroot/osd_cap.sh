#!/bin/sh

XWD_HEADER_SIZE=3179
XWD_FILE=/tmp/cap.xwd
CAPS_DIR=/caps
NX_MODEL="$1"

is_nx1() {
    [ "$NX_MODEL" == "NX1" ]
}

is_nx500() {
    [ "$NX_MODEL" == "NX500" ]
}

is_nx1_or_nx500() {
    is_nx1 || is_nx500
}

get_active_window() {
    eval $(xdotool getactivewindow getwindowgeometry | tr '\n,x' ' ' |\
           awk '{ printf "active_window_id=%d\nactive_window_geometry=%dx%d+%d+%d\n", $2, $9, $10,$4, $5 }')
}

is_fullscreen() {
    geometry=$1
    [ "$geometry" == "720x480+0+0" ] || \
    [ "$geometry" == "480x800+0+0" ] || \
    [ "$geometry" == "1024x768+720+0" ]
}

get_fullscreen_size() {
    if is_nx1_or_nx500; then
        echo 720x480
    else
        echo 480x800
    fi
}

get_screen_size() {
    if is_nx1; then
        echo 1744x768
    elif is_nx500; then
        echo 720x480
    else
        480x800
    fi
}

get_size() {
    echo $1 | sed 's/\+.*//'
}

mkdir -p $CAPS_DIR
get_active_window

if is_fullscreen $active_window_geometry; then
    md5=$(xwd -id $active_window_id | tee $XWD_FILE | md5sum | sed 's/  -//')
    window_size=$(get_size "$active_window_geometry")+$XWD_HEADER_SIZE
else
    md5=$(xwd -root | \
          convert -size $(get_screen_size)+$XWD_HEADER_SIZE \
                  -depth 8 bgra:- -crop $(get_fullscreen_size)+0+0 bgra:- | \
          tee $XWD_FILE | md5sum | sed 's/  -//')
    window_size=$(get_fullscreen_size)
fi
if [ -f $XWD_FILE ]; then
    file_size=$(wc -c $XWD_FILE | awk '{ print $1 }')
    if [ $file_size -eq 0 ]; then
        rm -f $XWD_FILE
        continue
    fi
fi

png_file=$CAPS_DIR/${active_window_geometry}_${md5}.png
if [ ! -f $png_file ]; then
    convert -size $window_size -depth 8 bgra:$XWD_FILE $png_file
fi
echo $png_file
