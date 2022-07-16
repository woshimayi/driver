#!/bin/bash

echo 1
time ./eff 1      < ../data1 2> /dev/null
echo 2
time ./eff 2      < ../data1 2> /dev/null
echo 4
time ./eff 4      < ../data1 2> /dev/null
echo 8
time ./eff 8      < ../data1 2> /dev/null
echo 16
time ./eff 16     < ../data1 2> /dev/null
echo 32
time ./eff 32     < ../data1 2> /dev/null
echo 64
time ./eff 64     < ../data1 2> /dev/null
echo 128
time ./eff 128    < ../data1 2> /dev/null
echo 256
time ./eff 256    < ../data1 2> /dev/null
echo 512
time ./eff 512    < ../data1 2> /dev/null
echo 1024
time ./eff 1024   < ../data1 2> /dev/null
echo 2048
time ./eff 2048   < ../data1 2> /dev/null
echo 4096
time ./eff 4096   < ../data1 2> /dev/null
echo 8192
time ./eff 8192   < ../data1 2> /dev/null
echo 16384
time ./eff 16384  < ../data1 2> /dev/null
echo 32768
time ./eff 32768  < ../data1 2> /dev/null
echo 65536
time ./eff 65536  < ../data1 2> /dev/null
echo 131072 
time ./eff 131072 < ../data1 2> /dev/null
