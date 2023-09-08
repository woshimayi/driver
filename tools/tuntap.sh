#/bin/sh
# 创建tap0 虚拟接口, 通过br0 桥 互联eth0 


if test -z $1
then
    echo need a argument: down/up
    exit 0
fi

if [ "up" = $1 ]
then
    # 新建一个网桥，名称为br0
    brctl addbr br0
    # 将网络设备eth0添加到网桥br0 
    ifconfig eth0 down
    brctl addif br0 eth0
    # 关闭生成树协议
    brctl stp br0 off

    ifconfig br0 10.8.8.1 netmask 255.255.255.0 promisc up
    ifconfig eth0 10.8.8.2 netmask 255.255.255.0 promisc up

    # 使用命令tunctl添加虚拟网卡tap
    ip tuntap add tap0 mode tap
    ip link set dev tap0 up
    ip addr add 10.8.8.4/24 dev tap0
    tunctl -t tap0 -u macrofun
    ifconfig tap0 10.8.8.4 netmask 255.255.255.0 promisc up

    brctl addif br0 tap0
else
    ifconfig tap0 down
    brctl delif br0 tap0

    ifconfig eth0 down
    brctl delif br0 eth0

    ifconfig br0 down
    brctl delbr br0

    ifconfig eth0 10.8.8.2 netmask 255.255.255.0
fi
