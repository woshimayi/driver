###
 # @*************************************: 
 # @FilePath: /user/home/zs/Documents/driver/tools/ncuress_shell_1.sh
 # @version: 
 # @Author: dof
 # @Date: 2023-07-19 16:41:59
 # @LastEditors: dof
 # @LastEditTime: 2023-07-19 16:49:22
 # @Descripttion: 
 # @**************************************: 
### 

#!/bin/bash
#!/bin/bash

# 引入ncurses库
source /usr/share/ncurses/termcap.src

# 初始化ncurses
clear
tput cup 2 5
echo "Welcome to Shell Ncurses Menu"

# 菜单选项
OPTIONS=("Option 1" "Option 2" "Option 3")

# 保存之前的设置
pre=$(stty -g)

# 创建菜单并处理用户选择
selected_option=$(dialog --clear --stdout --menu "Select an option:" 10 40 3 "${OPTIONS[@]}")

# 恢复之前的设置
stty $pre

# 清理屏幕并执行选择的选项
clear
echo "Selected Option: $selected_option"