###
# @*************************************:
 # @FilePath: /network/home/zs/Documents/driver/tools/disk_wirte_read_IOTEST.sh
# @version:
# @Author: dof
# @Date: 2023-09-08 16:19:13
 # @LastEditors: dof
 # @LastEditTime: 2023-09-08 16:44:01
# @Descripttion: 磁盘读写测试
# @**************************************:
###

#!/bin/bash
# Test disk write speed with dd, with different bs , 2GB data

BS=(512 1k 2k 4k 8k 16k 32k 64k 128k 256k 512k 1M 2M 4M 8M 12M 16M 24M 32M 48M 64M)
#COUNT_2G=(4000000 2000000 1000000 500000 250000 125000 62500 31250 15625 7813 3907 1954 977 489 245 163 123 82 62 41 31)
# reduce count for small BS < 4k
COUNT_2G=( 400000  200000  100000  500000 250000 125000 62500 31250 15625 7813 3907 1954 977 489 245 163 123 82 62 41 31)
FILE=file.tst

for id in $(seq ${#BS[*]}) ; do
    rm -rf $FILE 2>1 1>/dev/null
    sleep 3
    
    idx=`expr $id - 1`
    echo ---- $id: bs=${BS[$idx]},  ${COUNT_2G[$idx]}  --------
    dd if=/dev/zero of=$FILE bs=${BS[$idx]} count=${COUNT_2G[$idx]} oflag=direct
	dd if=$FILE of=/dev/null bs=${BS[$idx]}
done