#!/bin/sh

V=1.5.1
P=libjpeg-turbo-$V

if [ ! -d $P ]; then
    wget -c http://downloads.sourceforge.net/project/libjpeg-turbo/$V/$P.tar.gz
    tar xf $P.tar.gz
fi

cd $P
./configure --host=arm-none-linux-gnueabi --enable-static=yes --enable-shared=no
make -j8
cp -fv cjpeg djpeg jpegtran ../
