#!/bin/sh

P=busybox
V=1.25.0
PV=$P-$V

if [ ! -d $PV ]; then
    wget -c https://www.busybox.net/downloads/$PV.tar.bz2
    tar xf $PV.tar.bz2
fi

cp -f busybox.config $PV/.config
cd $PV
make -j8
cp -f busybox ..

