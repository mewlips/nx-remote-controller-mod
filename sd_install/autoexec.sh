#!/bin/sh

mkdir -p /dev/pts
mount -t devpts none /dev/pts

inetd /mnt/mmc/inetd.conf

/mnt/mmc/remote/connect.sh
