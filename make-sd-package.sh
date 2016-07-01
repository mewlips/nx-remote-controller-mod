#!/bin/sh

VERSION=$(cat version)
SD_ZIP_FILE=nx-remote-controller-mod-${VERSION}.zip

rm -f $SD_ZIP_FILE
(cd sd_install; zip -r ../$SD_ZIP_FILE info.tg remote/)
cp -f NXRemoteController/app/build/outputs/apk/app-debug.apk NXRemoteController-$VERSION.apk
