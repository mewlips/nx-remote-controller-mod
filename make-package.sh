#!/bin/sh

VERSION=$(cat version)

ZIP_FILE=nx-remote-controller-mod-v${VERSION}_nx1_nx500.zip
SD_ZIP_FILE=nx-remote-controller-mod-v${VERSION}_nx300.zip
NX_KS2_ZIP_FILE=nx-ks2_nx-rc-${VERSION}.zip

NX500_NX1_MODDING=externals/nx500_nx1_modding
EXTERNALS=install/app/externals
INSTALL_WEB_ROOT=install/app/web_root
SD_INSTALL_WEB_ROOT=sd_install/remote/web_root
NX_KS2_WEB_ROOT=nx-ks2/nx-rc/web_root

rm -fv $ZIP_FILE
rm -fv $SD_ZIP_FILE

mkdir -pv $EXTERNALS
cp -fv $NX500_NX1_MODDING/poker $EXTERNALS/

mkdir -pv $INSTALL_WEB_ROOT $SD_INSTALL_WEB_ROOT $NX_KS2_WEB_ROOT
cp -rfv web_root/{fonts,js,lib,index.html} $INSTALL_WEB_ROOT/
cp -rfv web_root/{fonts,js,lib,index.html} $SD_INSTALL_WEB_ROOT/
cp -rfv web_root/{fonts,js,lib,index.html} $NX_KS2_WEB_ROOT/

(cd install; zip -r ../$ZIP_FILE info.tg nx_cs.adj install.sh app/ )
(cd sd_install; zip -r ../$SD_ZIP_FILE autoexec.sh remote/)
(cd nx-ks2; zip -r ../$NX_KS2_ZIP_FILE nx-rc.sh off_nx-rc.sh nx-rc/)
if [ -f NXRemoteController/app/build/outputs/apk/app-debug.apk ]; then
    cp -fv NXRemoteController/app/build/outputs/apk/app-debug.apk NXRemoteController-v$VERSION.apk
fi
