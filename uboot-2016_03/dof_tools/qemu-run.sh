# 终端启动 
#iqemu-system-arm -M vexpress-a9 -m 512M -kernel u-boot -nographic

LAN=tap0

if [ -z "$1" ]
then
	echo "no str"
elif test '-d' = "$1"
then
	ip addr flush dev $LAN
	ip link set dev $LAN down
	ip tuntap del mode tap dev $LAN
	echo "delete tap $LAN"
	exit 0
elif test '-g' = "$1"
then 
	qemu-system-arm -M vexpress-a9 -m 256M -nographic -kernel u-boot -gdb tcp::1234  -S
	exit 0
elif test '-l' = "$1"
then 
	arm-linux-gnueabihf-gdb  -tui ../u-boot
	exit 0
fi





ip tuntap add $LAN mode tap
ip link set dev $LAN up
ip addr add 10.8.8.4/24 dev $LAN


# 使用 tap 使能网络接口功能
qemu-system-arm \
	-M vexpress-a9 -m 512 -nographic  \
	-net nic -net tap,ifname=$LAN,script=no \
	-kernel ../u-boot 

ip addr flush dev $LAN
ip link set dev $LAN down
ip tuntap del mode tap dev $LAN
