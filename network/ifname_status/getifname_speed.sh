#!/bin/bash

# 获取网卡名称
interface=$1

# 使用 ethtool 获取网卡信息
ethtool_output=$(ethtool $interface)

# 提取最大速率
max_speed=$(echo "$ethtool_output" | grep "Supported link modes" | awk '{print $NF}')

# 将最大速率赋值给变量
speed=$max_speed

# 打印最大速率
echo "最大速率: $speed"


