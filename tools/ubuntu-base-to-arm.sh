###
 # @*************************************: 
 # @FilePath     : \undefinedt:\Documents\driver\tools\ubuntu-base-to-arm.sh
 # @version      : 
 # @Author       : dof
 # @Date         : 2025-08-08 13:32:27
 # @LastEditors  : dof
 # @LastEditTime : 2025-08-08 13:32:27
 # @Descripttion :  移植ubuntu-base 到 imx6ull 开发板中
 # @compile      :  
 # @**************************************: 
### 


suod apt install qemu-user-static
sudo cp /usr/bin/qemu-arm-static usr/bin/

vim mount_self.sh
chmod 777 mount_self.sh

echo "#!/bin/bash"                                                                     > mount_self.sh
echo ""                                                                               >> mount_self.sh
echo "echo "MOUNTING""                                                                >> mount_self.sh
echo "sudo mount -t proc  /proc    /home/${whoami}/linux/nfs/ubuntu_rootfs/proc"      >> mount_self.sh                   
echo "sudo mount -t sysfs /sys     /home/${whoami}/linux/nfs/ubuntu_rootfs/sys"       >> mount_self.sh                  
echo "sudo mount -o bind  /dev     /home/${whoami}/linux/nfs/ubuntu_rootfs/dev"       >> mount_self.sh                 
echo "sudo mount -o bind  /dev/pts /home/${whoami}/linux/nfs/ubuntu_rootfs/dev/pts"   >> mount_self.sh                         
echo "sudo chroot /home/${whoami}/linux/nfs/ubuntu_rootfs"                            >> mount_self.sh 


vim unmount_self.sh
chmod 777 unmount_self.sh

echo "#!/bin/bash"                                                    > unmount_self.sh
echo "echo "UNMOUNTING""                                             >> unmount_self.sh
echo "sudo umount /home/${whoami}/linux/nfs/ubuntu_rootfs/proc"      >> unmount_self.sh
echo "sudo umount /home/${whoami}/linux/nfs/ubuntu_rootfs/sys"       >> unmount_self.sh
echo "sudo umount /home/${whoami}/linux/nfs/ubuntu_rootfs/dev/pts"   >> unmount_self.sh
echo "sudo umount /home/${whoami}/linux/nfs/ubuntu_rootfs/dev"       >> unmount_self.sh



echo "deb http://mirrors.tuna.tsinghua.edu.cn/ubuntu-ports/ jammy main restricted universe multiverse"                   > /etc/apt/sources.list
echo "deb http://mirrors.tuna.tsinghua.edu.cn/ubuntu-ports/ jammy-updates main restricted universe multiverse"          >> /etc/apt/sources.list
echo "deb http://mirrors.tuna.tsinghua.edu.cn/ubuntu-ports/ jammy-backports main restricted universe multiverse"        >> /etc/apt/sources.list
echo "deb http://mirrors.tuna.tsinghua.edu.cn/ubuntu-ports/ jammy-security main restricted universe multiverse"         >> /etc/apt/sources.list

./mount_self.sh

ln -s /lib/systemd/system/getty@.service /etc/systemd/system/getty.target.wants/getty@ttymxc0.service

apt update; apt install vim net-tools openserver-ssh htop

./unmount_self.sh