#! /bin/sh
rm -rf PROGRAMMING.bin
dd if=$1 of=PROGRAMMING.bin bs=1k conv=sync
dd if=$2 of=PROGRAMMING.bin bs=1k seek=512
dd if=$3 of=PROGRAMMING.bin bs=1k seek=3584 conv=sync
dd if=$4 of=PROGRAMMING.bin bs=1k seek=6144 conv=sync
#0.5  3   2.5 10 
echo ''
echo 'flash.bin address map'
echo '0x00000000 : u-boot'
echo '0x00100000 : Linux Kernel'
echo '0x00200000 : JFFS2'
echo '0x00600000 : APP '