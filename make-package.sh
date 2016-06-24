#!/bin/sh

ZIP_FILE=nx-remote-controller-mod.zip

rm -f $ZIP_FILE
(cd install; zip -r ../$ZIP_FILE app/ nx-on-wake/)
cp -f NXRemoteController/app/build/outputs/apk/app-debug.apk NXRemoteController.apk
