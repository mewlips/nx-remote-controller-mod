#!/bin/sh

V=0.6.21
P=exif-$V

if [ ! -d $P ]; then
    #wget -c http://downloads.sourceforge.net/project/exif/$V/$P.tar.gz
    tar xf $P.tar.bz2
    cp libpopt.so.0.0.0 $P/exif/libpopt.so
    cp libexif.so.12.3.3 $P/exif/libexif.so
    cp -r libexif popt.h $P/exif/
    cp -r libexif $P/libjpeg/
fi

cd $P

POPT_CFLAGS="-I. " POPT_LIBS="-L. -lpopt" ./configure --host=arm-none-linux-gnueabi 
make -j8
cp exif/exif ..
