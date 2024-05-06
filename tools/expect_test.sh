#!/usr/bin/expect
###
 # @*************************************: 
 # @FilePath: \tools\expect_test.sh
 # @version: 
 # @Author: dof
 # @Date: 2024-02-04 14:27:41
 # @LastEditors: dof
 # @LastEditTime: 2024-02-04 14:49:35
 # @Descripttion: 自动ssh登录执行命令
 # @**************************************: 
### 
set ip 172.16.27.16
set pass zx
set timeout 5
spawn ssh zs@$ip
expect {
    "yes/no" { send "yes\r";exp_continue }
    "password:" { send "$pass\r" }
}

expect "$ "
send "cd wget\n"
expect "$ "
send "touch 123zzzzz\n"
expect "$ "
send "date\n"
expect "$ "
send "exit\n"
expect eof
# interact      # 交互模式,用户会停留在远程服务器上面