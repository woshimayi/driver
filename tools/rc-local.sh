#!/bin/bash
# ubuntu 生成rc-local 自启动脚本

service_name="/etc/systemd/system/rc-local.service"
#service_name="rc-local.service"

echo "[Unit]" >> $service_name
echo "Description=/etc/rc.local Compatibility" >> $service_name
echo "ConditionPathExists=/etc/rc.local" >> $service_name
echo ""  >> $service_name
echo "[Service]" >> $service_name
echo "Type=forking" >> $service_name
echo "ExecStart=/etc/rc.local start" >> $service_name
echo "TimeoutSec=0" >> $service_name
echo "StandardOutput=tty" >> $service_name
echo "RemainAfterExit=yes" >> $service_name
echo "SysVStartPriority=99" >> $service_name
echo " " >> $service_name
echo "[Install]" >> $service_name
echo "WantedBy=multi-user.target" >> $service_name


rclocal="/etc/rc.local"
# rclocal="rc.local"

echo "#!/bin/sh -e" >> $rclocal
echo "#" >> $rclocal
echo "# rc.local" >> $rclocal
echo "#" >> $rclocal
echo "# This script is executed at the end of each multiuser runlevel." >> $rclocal
echo "# Make sure that the script will "exit 0" on success or any other" >> $rclocal
echo "# value on error." >> $rclocal
echo "#" >> $rclocal
echo "# In order to enable or disable this script just change the execution" >> $rclocal
echo "# bits." >> $rclocal
echo "#" >> $rclocal
echo "# By default this script does nothing." >> $rclocal
echo "echo "看到这行字，说明添加自启动脚本成功。" > /usr/local/test.log" >> $rclocal
echo "exit 0" >> $rclocal


#chmod +x /etc/rc.local
#systemctl enable rc-local

#systemctl start rc-local.service
#systemctl status rc-local.service
