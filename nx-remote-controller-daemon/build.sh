#!/bin/sh

arm-none-linux-gnueabi-gcc \
        -DDEBUG -O4 -Wall -lpthread \
        -o nx-remote-controller-daemon \
        command.c \
        executor.c \
        network.c \
        notify.c \
        main.c \
        nx_model.c \
        util.c \
        video.c \
        xwin.c \
    && cp -fv nx-remote-controller-daemon ../install/app/ \
    && cp -fv nx-remote-controller-daemon ../sd_install/remote/
