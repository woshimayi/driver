#!/bin/sh
sudo apt-get install bison build-essential gperf flex ruby python libasound2-dev libbz2-dev libcap-dev libcups2-dev libdrm-dev libegl1-mesa-dev libgcrypt11-dev libnss3-dev libpci-dev libpulse-dev libudev-dev libxtst-dev gyp ninja-build libglu1-mesa-dev libfontconfig1-dev libx11-xcb-dev libicu-dev

xz -d https://download.qt.io/development_releases/qt/5.15/5.15.0-rc2/single/qt-everywhere-src-5.15.0-rc2.tar.xz
tar -xvf qt-everywhere-src-5.15.0-rc2.tar
cd qt-everywhere-src-5.15.0-rc2/
./configure
make 
make install


