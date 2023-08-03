#/bin/sh
if test -z $1
then
    echo need a argument: down/up
    exit 0
fi

LAN=$1
STATE=$2

if [ "up" = $STATE ]
then
	#ip tuntap add tap0 mode tap
	#ip link set dev tap0 up
	#ip addr add 10.8.8.2/24 dev tap0

    # 新建一个网桥，名称为br0
    #brctl addbr br0
    # 将网络设备ens38添加到网桥br0 
    #ifconfig $LAN down
    #brctl addif br0 $LAN
    # 关闭生成树协议
    #brctl stp br0 off

    # ifconfig br0 10.8.8.3 netmask 255.255.255.0 promisc up
    ifconfig $LAN 10.8.8.4 netmask 255.255.255.0 promisc up
    # 使用命令tunctl添加虚拟网卡tap
    # tunctl -t tap0 -u macrofun
    # ifconfig tap0 10.8.8.4 netmask 255.255.255.0 promisc up
    #brctl addif br0 tap0
else
    #ifconfig tap0 down
    #brctl delif br0 tap0

    #ifconfig $LAN down
    #brctl delif br0 $LAN


    #ifconfig br0 down
    #brctl delbr br0

    ifconfig $LAN 10.8.8.4 netmask 255.255.255.0

    #ip addr flush dev tap0
    #ip link set dev tap0 down 
    #ip tuntap del mode tap dev tap0
fi
