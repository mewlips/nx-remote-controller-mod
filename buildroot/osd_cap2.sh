#!/bin/sh

NX_MODEL="$1"
RESIZE="$2"
XWD_HEADER_SIZE=3179

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
        echo 480x800
    fi
}

get_size() {
    echo $1 | sed 's/\+.*//'
}

reduce_size() {
    local size=$1
    echo $size | sed 's/x/ /' | awk '{ printf "%0.fx%0.f", $1 / 2, $2 / 2}'
}

convert_to_png() {
    local size=$(get_size $1)
    local out_opts
    if is_fullscreen $active_window_geometry; then
        out_opts="-crop $active_window_geometry"
    else
        out_opts="-crop $(get_fullscreen_size)"
    fi
    if [ "$RESIZE" = "sample" ]; then
        out_opts="$out_opts -sample $(reduce_size $(get_fullscreen_size))"
    elif [ "$RESIZE" = "scale" ]; then
        out_opts="$out_opts -scale $(reduce_size $(get_fullscreen_size))"
    fi
    if ! is_nx1_or_nx500; then
        out_opts="$out_opts -rotate 90"
    fi
    convert -size $size+$XWD_HEADER_SIZE -depth 8 bgra:- $out_opts -quality 20 png:-
}

capture() {
    get_active_window

    if [ "$active_window_geometry" = "1024x768+720+0" ]; then
        xwd -root | convert_to_png $(get_screen_size)
    elif is_fullscreen $active_window_geometry; then
        xwd -id $active_window_id | convert_to_png $active_window_geometry
    else
        xwd -root | convert_to_png $(get_screen_size)
    fi
}

capture
#xwd -id 16777219 | convert -size 720x480+3179 -depth 8 bgra:- -crop 720x480+0+0 -sample 360x240 -quality 10 png:-
