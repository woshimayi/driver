#!/bin/bash

CURRENT_USER=$(whoami)

echo "--- 系统管理终端 ---"
echo "登录用户: $CURRENT_USER"
CURRENT_USER="zzzzz"

user_auth() {
    case $1 in
        zzzzz|xxxxx|ccccc|vvvvv)
            echo "权限级别: simple user $CURRENT_USER"
            exit 1
            ;;
        *)
            echo "权限级别： (Root)"
            ;;
    esac
}



case "$opt" in
    "get")
        echo "get 当前用户: $CURRENT_USER"
        ;;
    "show")
        echo "show 当前用户: $CURRENT_USER"
        ;;
    "rm")
        echo "rm 当前用户: $CURRENT_USER"
        user_auth "$CURRENT_USER"
        ;;
    *)
        echo "未知操作: $opt"
        user_auth "$CURRENT_USER"
        ;;
esac
