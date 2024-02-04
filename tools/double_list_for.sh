#!/bin/sh

list1="apple banana cherry"
list2="red yellow pink"

# 获取两个列表的长度
length=${#list1[@]}


# 使用for循环同时遍历两个列表
for ((i=0; i<$length; i++))
do
    item1=${list1[$i]}
    item2=${list2[$i]}
    echo "Item 1: $item1, Item 2: $item2"
done