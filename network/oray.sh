#/bin/bash


# 可以通过wget 判断 ddns 是否注册成功


# 花生壳用户名
username='zzzzz'
# 花生壳密码
password='zzzzz'
# 花生壳申请的域名
yourhostname='zzzzz'

ret=`wget "http://$username:$password@ddns.oray.com/ph/update?hostname=$yourhostname"`

echo $ret