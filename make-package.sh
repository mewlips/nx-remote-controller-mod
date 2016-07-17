#!/bin/sh

VERSION=$(cat version)
ZIP_FILE=nx-remote-controller-mod-v${VERSION}_nx500.zip
SD_ZIP_FILE=nx-remote-controller-mod-v${VERSION}_nx300.zip

rm -f $ZIP_FILE
rm -f $SD_ZIP_FILE
(cd install; zip -r ../$ZIP_FILE app/ nx-on-wake/)
(cd sd_install; zip -r ../$SD_ZIP_FILE autoexec.sh remote/)
cp -f NXRemoteController/app/build/outputs/apk/app-debug.apk NXRemoteController-v$VERSION.apk
