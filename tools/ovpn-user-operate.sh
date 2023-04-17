#!/bin/bash
:<<!
【脚本说明】
1、此脚本适用操作openvpn用户；
2、支持用户创建、删除；
!

# openvpn配置目录
cfdir=/data/.openvpn
# openvpn 服务名
ovname=openvpn-wlf
# 设置的私钥密码
spwd=12345678
# 提示信息，需手动复制私钥密码
msg="Pass phrase: $spwd"

# 服务基本信息
operate=$1
username=$2

# 提示信息
msg="Please input the param [<new|del> <username>]"

# 定制化shell输出
function custom_print() {
    echo -e "\033[5;34m ***** \033[0m"
    echo -e "\033[32m $@ ! \033[0m"
    echo -e "\033[5;34m ***** \033[0m"
}

# 判断输入参数
if [[ -z $operate || -z $username ]]; then
    custom_print $msg

# 启动服务
elif [[ $operate = "new" || $operate = "create" ]]; then
    custom_print $msg
	mkdir -p $cfdir/conf
    docker run -v $cfdir:/etc/openvpn --rm -it kylemanna/openvpn  easyrsa build-client-full $username nopass
    docker run -v $cfdir:/etc/openvpn --rm kylemanna/openvpn  ovpn_getclient $username > $cfdir/conf/"$username".ovpn
    sed -i '$s/.*/comp-lzo no/' $cfdir/conf/"$username".ovpn
    docker restart $ovname > /dev/null
    msg="Create user 【$username】 success"
    custom_print $msg
    msg="The user conf is 【$cfdir/conf/"$username".ovpn】"
    custom_print $msg

# 停止服务
elif [[ $operate = "del" || $operate = "delete" ]]; then
    custom_print $msg
    docker run -v $cfdir:/etc/openvpn --rm -it kylemanna/openvpn  easyrsa revoke $username
    docker run -v $cfdir:/etc/openvpn --rm -it kylemanna/openvpn  easyrsa gen-crl
    docker run -v $cfdir:/etc/openvpn --rm -it kylemanna/openvpn  rm -f /etc/openvpn/pki/reqs/"$username".req
    docker run -v $cfdir:/etc/openvpn --rm -it kylemanna/openvpn  rm -f /etc/openvpn/pki/private/"$username".key
    docker run -v $cfdir:/etc/openvpn --rm -it kylemanna/openvpn  rm -f /etc/openvpn/pki/issued/"$username".crt
    rm -rf $cfdir/conf/"$username".ovpn
    docker restart $ovname > /dev/null
    msg="Delete user 【$username】 success"
    custom_print $msg

else
    custom_print $msg
fi