#!/bin/bash
###
 # @*************************************: 
 # @FilePath: /user/home/zs/Documents/driver/tools/ncuress_shell.sh
 # @version: 
 # @Author: dof
 # @Date: 2023-07-19 16:33:34
 # @LastEditors: dof
 # @LastEditTime: 2023-07-19 16:39:30
 # @Descripttion:  shell 实现 ncuress 功能
 # @**************************************: 
### 

# 引入ncurses库
source /usr/share/ncurses/termcap.src


# 初始化ncurses
clear
tput cup 2 5
echo "Welcome to Shell Ncurses Demo"
tput cup 3 5
echo "Press 'q' to exit"
tput cup 5 5

# 开启curses模式
stty -echo
tput smkx

# 保存之前的设置
pre=$(stty -g)

# 缓冲区设置
line=""
buf=""

# 循环读取用户输入
while true; do
  IFS= read -s -n 1 key
  case "$key" in
    $'\x0a')  # 回车键
      line=$buf  # 保存输入行
      tput cud1  # 光标下移1行
      tput el    # 清除当前行
      tput cup 5 5
      echo "You entered: $line"
      buf=""     # 清空缓冲区
      ;;
    $'\x7f')  # 删除键
      tput cub1  # 光标左移1列
      tput el    # 清除当前列
      buf=${buf%?}  # 删除缓冲区最后一个字符
      ;;
    q)        # 按下'q'键退出
      break
      ;;
    h)
      echo 'aaaaaaaaaaaaaaaa'
      echo 'bbbbbbbbbbbbbbbb'
    ;;
    *)        # 其他键
      buf+="$key"  # 将键值添加到缓冲区
      echo -n "$key"  # 在终端上显示键值
      ;;
  esac
done

# 恢复之前的设置
stty echo          # 用于重新启用终端回显
tput rmkx          # 用于禁用curses模式
stty $pre          # 用于恢复之前的终端设置


# 清理屏幕并退出
tput clear
stty sane         # 重置终端设置
exit 0