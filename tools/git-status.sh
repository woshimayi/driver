#!/bin/bash
###
# @*************************************:
 # @FilePath     : \undefinedw:\tools\git-status.sh
# @version      :
# @Author       : dof
# @Date         : 2025-06-03 14:47:18
 # @LastEditors  : dof
 # @LastEditTime : 2025-06-04 12:00:06
# @Descripttion :
# @compile      :
# @**************************************:
###
#git status $@ | grep modified | awk -v red="\033[31m" -v reset="\033[0m" '{print red "\t " $2 reset}'

# echo "xxxxx $0 $@"

# ln -sf git-status.sh gs
# ln -sf git-status.sh gf
# ln -sf git-status.sh gr
# ln -sf git-status.sh gl
# ln -sf git-status.sh gb
# ln -sf git-status.sh gp
# ln -sf git-status.sh gc


command=$(basename $0)

case $command in
    gf)
        git diff $@
        ;;
    gs)
        git -c color.status=always status $@ | grep --color=never -E "new file|modified" | sed 's/modified:[[:space:]]\+//' | sed 's/new file:[[:space:]]\+//'
        ;;
    gl)
        if [ $# -ne 0 ]; then
            git log --stat $@
        else
            git log --graph --pretty=format:'%Cred%h%Creset -%C(yellow)%d%Creset %s %Cgreen(%cr) %C(bold blue)<%an>%Creset' --abbrev-commit --date=relative -10
        fi
        ;;
    gb)
        git branch $@
        ;;
    gc)
        git commit
        ;;
    gr)
        git restore $@
        ;;
    gp)
        git pull $@
        ;;

esac
