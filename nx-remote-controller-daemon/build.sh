#!/bin/sh

export PATH=$PATH:/home/mewlips/devices/nx500/NX500_opensource/kernel/CodeSourcery/Sourcery_CodeBench_Lite_for_ARM_GNU_Linux/bin

arm-none-linux-gnueabi-gcc nx-remote-controller-daemon.c -DDEBUG -O4 -Wall -lpthread -o nx-remote-controller-daemon && \
     cp -fv nx-remote-controller-daemon ../install/app/
