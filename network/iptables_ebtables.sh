#!/bin/sh
###
 # @*************************************: 
 # @FilePath     : /user/home/zsD/Documents/driver/network/iptables_ebtables.sh
 # @version      : 
 # @Author       : dof
 # @Date         : 2024-12-28 18:39:57
 # @LastEditors  : dof
 # @LastEditTime : 2024-12-28 18:39:58
 # @Descripttion :  
 # @compile      :  
 # @**************************************: 
### 
#

#IFNAME="br0"
IFNAME="enp0s9"

_log_iptables() {
	local method
	
	if [ "$1" == "add" ]; then
		method="-I"
	else
		method="-D"
	fi
	
	iptables -t mangle $method PREROUTING -p icmp -j LOG --log-level 6 \
		--log-prefix "mangle:PREROUTING " >/dev/null 2>&1
		
	iptables -t mangle $method INPUT -p icmp -j LOG --log-level 6 \
		--log-prefix "mangle:INPUT " >/dev/null 2>&1
		
	iptables -t mangle $method FORWARD -p icmp -j LOG --log-level 6 \
		--log-prefix "mangle:FORWARD " >/dev/null 2>&1
		
	iptables -t mangle $method OUTPUT -p icmp -j LOG --log-level 6 \
		--log-prefix "mangle:OUTPUT " >/dev/null 2>&1

	iptables -t mangle $method POSTROUTING -p icmp -j LOG --log-level 6 \
		--log-prefix "mangle:POSTROUTING " >/dev/null 2>&1
}

flush_log_iptables() {
	_log_iptables "del"
}

setup_log_iptables() {
	#iptables -t raw -A OUTPUT -p icmp -j TRACE
	#iptables -t raw -A PREROUTING -p icmp -j TRACE
	
	_log_iptables "add"
}

_log_ebtables() {
	local method
	
	if [ "$1" == "add" ]; then
		method="-I"
	else
		method="-D"
	fi
	
	ebtables -t broute $method BROUTING -p ipv4 --ip-proto 1 --log-level 6 --log-ip \
		--log-prefix "[EBT]broute:BROUTING" -j ACCEPT
		
	ebtables -t nat $method OUTPUT -p ipv4 --ip-proto 1 --log-level 6 --log-ip \
		--log-prefix "[EBT]nat:OUTPUT"  -j ACCEPT
		
	ebtables -t nat $method PREROUTING -p ipv4 --ip-proto 1 --log-level 6 --log-ip \
		--log-prefix "[EBT]nat:PREROUTING" -j ACCEPT
		
	ebtables -t filter $method INPUT -p ipv4 --ip-proto 1 --log-level 6 --log-ip \
		--log-prefix "[EBT]filter:INPUT" -j ACCEPT
		
	ebtables -t filter $method FORWARD -p ipv4 --ip-proto 1 --log-level 6 --log-ip \
		--log-prefix "[EBT]filter:FORWARD" -j ACCEPT
		
	ebtables -t filter $method OUTPUT -p ipv4 --ip-proto 1 --log-level 6 --log-ip \
		--log-prefix "[EBT]filter:OUTPUT" -j ACCEPT
		
	ebtables -t nat $method POSTROUTING -p ipv4 --ip-proto 1 --log-level 6 --log-ip \
		--log-prefix "[EBT]nat:POSTROUTING" -j ACCEPT
}

flush_log_ebtables() {
	_log_ebtables "del"
}

setup_log_ebtables() {
	_log_ebtables "add"
}

flush_log_tc() {
	tc qdisc del dev $IFNAME root
	tc qdisc del dev $IFNAME handle ffff: ingress

	tc qdisc del dev ifb0 root
	tc qdisc add dev ifb0 root handle 1: htb

	tc qdisc del dev ifb1 root
	tc qdisc add dev ifb1 root handle 1: htb
}

setup_log_tc() {
	tc qdisc add dev ifb0 root handle 1: htb default 2
	
	tc class add dev ifb0 parent 1: classid 1:1 htb rate 2Mbit
	tc class add dev ifb0 parent 1: classid 1:2 htb rate 10Mbit
	
	tc filter add dev ifb0 parent 1: protocol ip prio 1 u32 \
		match ip protocol 1 0xff flowid 1:1 \
		action simple "tc[ifb0]egress"
		
	tc qdisc add dev ifb0 ingress
	
	tc filter add dev ifb0 parent ffff: protocol ip prio 1 u32 \
		match ip protocol 1 0xff \
		action simple "tc[ifb0]ingress"

	tc qdisc add dev ifb1 root handle 1: htb default 2
	
	tc class add dev ifb1 parent 1: classid 1:1 htb rate 2Mbit
	tc class add dev ifb1 parent 1: classid 1:2 htb rate 10Mbit
	
	tc filter add dev ifb1 parent 1: protocol ip prio 1 u32 \
		match ip protocol 1 0xff flowid 1:1 \
		action simple "tc[ifb1]egress"
		
	tc qdisc add dev ifb1 ingress
	
	tc filter add dev ifb1 parent ffff: protocol ip prio 1 u32 \
		match ip protocol 1 0xff \
		action simple "tc[ifb1]ingress"			
		
	tc qdisc add dev $IFNAME root handle 1: htb default 2
	
	tc class add dev $IFNAME parent 1: classid 1:1 htb rate 2Mbit
	tc class add dev $IFNAME parent 1: classid 1:2 htb rate 10Mbit
	
	tc filter add dev $IFNAME parent 1: protocol ip prio 1 u32 \
		match ip protocol 1 0xff flowid 1:1 \
		action simple "tc[$IFNAME]egress" pipe \
		action mirred egress redirect dev ifb0
		
	tc qdisc add dev $IFNAME ingress
	
	tc filter add dev $IFNAME parent ffff: protocol ip prio 1 u32 \
		match ip protocol 1 0xff \
		action simple "tc[$IFNAME]ingress" pipe \
		action mirred egress redirect dev ifb1	
}

_flush_log() {
	flush_log_iptables
	flush_log_ebtables
	flush_log_tc
}

_setup_log() {
	_flush_log
	
	setup_log_iptables
	setup_log_ebtables
	setup_log_tc
}

main() {
	local argc=$1; shift
	local action=$1
	
	[ "$(whoami)" != "root" ] && {
		echo "Need root user to execute !!!"
		exit 1
	}
	
	rm -f /tmp/tc.log
	
	case "$action" in
		"")
			_setup_log
		;;
		flush)
			_flush_log
		;;
	esac
}

main $# $*

exit 0