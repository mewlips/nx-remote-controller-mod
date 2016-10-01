#!/bin/bash

DCIM_DIR=/sdcard/DCIM
GALLERY_DIR=/sdcard/gallery

DCRAW=$GALLERY_DIR/dcraw
CJPEG=$GALLERY_DIR/cjpeg
DJPEG=$GALLERY_DIR/djpeg
EXIF=$GALLERY_DIR/exif
UNIQ="$GALLERY_DIR/busybox uniq"

THUMB_DIR=$GALLERY_DIR/thumbs
mkdir -p $THUMB_DIR

INFO_JSON=$GALLERY_DIR/info.json

names=$(for f in $DCIM_DIR/*/*.{SRW,JPG,MP4}; do
    echo ${f/.*}
done | sort | $UNIQ)

dirs=$(for name in $names; do
    echo $THUMB_DIR/$(dirname ${name#/sdcard})
done | sort | $UNIQ)
mkdir -p $dirs

process_srw() {
    local name=$1
    local jpg=$THUMB_DIR/${name#/sdcard}.SRW.jpg
    local thumb_jpg=$THUMB_DIR/${name#/sdcard}.SRW_thumb.jpg
    local info=$THUMB_DIR/${name#/sdcard}.info
    if [ ! -f $jpg ]; then
        $DCRAW -e -c $name.SRW > $jpg
    fi
    if [ ! -f $thumb_jpg ]; then
        $DJPEG -scale 1/16 -bmp $jpg | $CJPEG > $thumb_jpg
    fi
    if [ ! -f $THUMB_DIR/$name.info ]; then
        $DCRAW -i -v $name.SRW > $info
    fi
}

process_jpg() {
    local name=$1
    local thumb_jpg=$THUMB_DIR/${name#/sdcard}.JPG_thumb.jpg
    local info=$THUMB_DIR/${name#/sdcard}.info
    if [ ! -f $thumb_jpg ]; then
        $EXIF -e -o $thumb_jpg $name.JPG
    fi
    if [ ! -f $info ]; then
        $EXIF $name.JPG > $info
    fi
}

process_srw_jpg() {
    local_name=$1
    local thumb_jpg=$THUMB_DIR/${name#/sdcard}.SRW_JPG_thumb.jpg
    local info=$THUMB_DIR/${name#/sdcard}.info
    if [ ! -f $thumb_jpg ]; then
        $EXIF -e -o $thumb_jpg $name.JPG
    fi
    if [ ! -f $info ]; then
        $DCRAW -i -v $name.SRW > $info
        $EXIF $name.JPG >> $info
    fi
}

process_mp4() {
    local_name=$1
}

for name in $names; do
    if [ -f $name.SRW ] && [ -f $name.JPG ]; then
        process_srw_jpg $name
        echo $name : SRW and JPG
    elif [ -f $name.SRW ]; then
        process_srw $name
        echo $name : SRW
    elif [ -f $name.JPG ]; then
        process_jpg $name
        echo $name : JPG
    elif [ -f $name.MP4 ]; then
        process_mp4 $name
        echo $name : MP4
    fi
done

exit
