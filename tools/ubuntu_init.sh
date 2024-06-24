#!/bin/bash
#
apt update
apt vim git htop range net-tools openssh-server samba samba-common build-essential -y
apt-get install language-pack-zh-hans
apt-get install fonts-droid-fallback ttf-wqy-zenhei ttf-wqy-microhei fonts-arphic-ukai fonts-arphic-uming



echo 'LANG="zh_CN.UTF-8' >> /etc/environment
echo 'LANGUAGE="zh_CN:zh:en_US:en"' >> /etc/environment
echo 'en_US.UTF-8 UTF-8' >> /var/lib/locales/supported.d/local
echo 'zh_CN.UTF-8 UTF-8' >> /var/lib/locales/supported.d/local
echo 'zh_CN.GBK GBK' >> /var/lib/locales/supported.d/local
echo 'zh_CN GB2312' >> /var/lib/locales/supported.d/local

locale-gen

update-alternatives --config editor


echo 'GRUB_CMDLINE_LINUX="net.ifnames=0 biosdevname=0"' >> /etc/default/grub

update-grub
cp .vimrc ~/
wget https://github.com/charmbracelet/glow/releases/download/v1.4.1/glow_1.4.1_linux_amd64.deb
apt install ./glow_1.4.1_linux_amd64.deb
