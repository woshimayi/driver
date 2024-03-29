#!/bin/sh
###
# @*************************************:
 # @FilePath: \undefinedx:\Documents\driver\tools\wand_test.sh
# @version:
# @Author: dof
# @Date: 2022-01-28 15:40:46
 # @LastEditors: dof
 # @LastEditTime: 2022-01-30 11:41:46
# @Descripttion:
# @**************************************:
###

# head="Content-Type: application/json"

ip="172.16.27.129"
head="application/x-www-form-urlencoded; charset=UTF-8"

action_1='{"cmdType":"wanConnection","para":{"wanIndex":[1]}}'
action_2='{"cmdType":"wanConnection","para":{"wanIndex":[2]}}'

add_obj='{"path":"hbus://wan/editWan","para":{"WanName":"create_new_wan","Enable":true,"LanInterface":"","VLANMode":2,"VLANIDMark":6,"802-1pMark":0,"LanInterface-DHCPEnable":true,"IPMode":1,"NATEnabled":true,"MTU":1460,"ConnectionType":"IP_Routed","AddressingType":"DHCP","ServiceList":"INTERNET"}}'
add_tr69='{"path":"hbus://wan/editWan","para":{"WanName":"create_new_wan","Enable":true,"LanInterface":"","VLANMode":2,"VLANIDMark":6,"802-1pMark":0,"MulticastVlan":-1,"IPMode":1,"MTU":1460,"ConnectionType":"IP_Routed","AddressingType":"DHCP","ServiceList":"TR069"}}'
add_voip='{"path":"hbus://wan/editWan","para":{"WanName":"create_new_wan","Enable":true,"LanInterface":"","VLANMode":2,"VLANIDMark":6,"802-1pMark":0,"IPMode":1,"MTU":1460,"ConnectionType":"IP_Routed","AddressingType":"DHCP","ServiceList":"VOIP"}}'
edit_ipoe='{"path":"hbus://wan/editWan","para":{"WanName":"1","Enable":true,"LanInterface":"","VLANMode":2,"VLANIDMark":6,"802-1pMark":0,"MulticastVlan":-1,"LanInterface-DHCPEnable":true,"IPMode":1,"NATEnabled":true,"MTU":1500,"ConnectionType":"IP_Routed","AddressingType":"DHCP","ServiceList":"INTERNET"}}'
edit_pppoe='{"path":"hbus://wan/editWan","para":{"WanName":"2","Enable":true,"LanInterface":"","VLANMode":2,"VLANIDMark":200,"802-1pMark":0,"MulticastVlan":-1,"LanInterface-DHCPEnable":true,"IPMode":1,"NATEnabled":true,"MTU":1460,"ConnectionType":"PPPoE_Routed","Username":"test16","Password":"test16","ConnectionTrigger":"AlwaysOn","ServiceList":"INTERNET"}}'

# get wan name
get="http://172.16.27.129/getHbusData?path=hbus://wan/getWan&msgType=213&userTagData=1"

# post
add="http://172.16.27.129/setHbusData?path=hbus://wan/editWan&msgType=213&userTagData=2"
# post wan info
get_info="http://172.16.27.129/setHbusData?path=hbus://wan/getWan&msgType=213&userTagData=3&waitTimeoutMs=5000&wanIndex=1"

# post
del="http://172.16.27.129/setHbusData?path=hbus://wan/editWan&msgType=213&userTagData=4&waitTimeoutMs=5000&wanIndex=1"

edit='http://172.16.27.129/setHbusData?path=hbus://wan/editWan&msgType=213&userTagData=2'


while true; do
	# del wan
	for i in $(seq 1 8); do
		# get wan name
		curl -s -H $head $get --insecure | jq '.'
		getwanname=$(curl -s -H $head $get --insecure | jq '.result[0] | .Index')
		token=$(echo ''"${getwanname}"'' | tr -d '"')
		echo "token:" $token
		if [ -n "$token" ]; then
			del_obj=$(echo $action_1 | jq '. |= . + {"para":{"wanIndex":['$token']}}' | tr -d ' \n')
			del_url="http://172.16.27.129/setHbusData?path=hbus://wan/editWan&msgType=213&userTagData=4&waitTimeoutMs=5000&wanIndex=$token"
			echo "del url $del_obj $del_url"
			echo "del ing ... "
			curl -H $head -X POST -d $del_obj  $del_url
			echo "del success ... "
			continue
		fi
	done

	# add wan
	for i in $add_obj $add_tr69 $add_voip $edit_ipoe $add_pppoe; do
		echo "create wan ing... " $i
		curl -H $head -X POST -d $i $add
		sleep 10
		curl -s -H $head $get --insecure | jq '.'
		echo "create wan success "
	done

	for i in $(seq 1 20); do
		# get wan info
		curl -H $head -X POST -d $action_1 $get_info --insecure | jq '.'
		curl -H $head -X POST -d $action_2 $get_info --insecure | jq '.'
	done

done
