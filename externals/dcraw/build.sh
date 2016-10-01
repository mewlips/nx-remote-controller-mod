#!/bin/sh

arm-none-linux-gnueabi-gcc -O4 -lm -DNODEPS -o dcraw dcraw.c
