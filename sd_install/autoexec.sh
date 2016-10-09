#!/bin/sh

mkdir -p /dev/pts
mount -t devpts none /dev/pts

inetd /mnt/mmc/inetd.conf

/mnt/mmc/remote/start.sh
/mnt/mmc/remote/connect.sh
