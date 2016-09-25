#!/bin/sh

VERSION=$(cat version)
ZIP_FILE=nx-remote-controller-mod-v${VERSION}_nx1_nx500.zip
SD_ZIP_FILE=nx-remote-controller-mod-v${VERSION}_nx300.zip
NX500_NX1_MODDING=externals/nx500_nx1_modding
EXTERNALS=install/app/externals
INSTALL_WEB_ROOT=install/app/web_root
SD_INSTALL_WEB_ROOT=sd_install/remote/web_root

rm -fv $ZIP_FILE
rm -fv $SD_ZIP_FILE

mkdir -pv $EXTERNALS
cp -fv $NX500_NX1_MODDING/poker $EXTERNALS/

mkdir -pv $INSTALL_WEB_ROOT $SD_INSTALL_WEB_ROOT
cp -fv web_root/*.{js,html} $INSTALL_WEB_ROOT/
cp -fv web_root/*.{js,html} $SD_INSTALL_WEB_ROOT/

(cd install; zip -r ../$ZIP_FILE info.tg nx_cs.adj install.sh app/ )
(cd sd_install; zip -r ../$SD_ZIP_FILE autoexec.sh remote/)
cp -fv NXRemoteController/app/build/outputs/apk/app-debug.apk NXRemoteController-v$VERSION.apk
