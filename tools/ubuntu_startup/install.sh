#!/bin/sh

cp rc.local /etc/
cp rc.local.service /lib/systemd/system
chmod +x /etc/rc.local
ln -s /lib/systemd/system/rc.local.service /etc/systemd/system/ 
