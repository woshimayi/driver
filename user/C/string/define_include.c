/*
 * @*************************************:
 * @FilePath: /user/C/define_include.c
 * @version:
 * @Author: dof
 * @Date: 2024-06-13 14:24:26
 * @LastEditors: dof
 * @LastEditTime: 2024-06-13 14:37:10
 * @Descripttion: 数据组初始化包含文件
 * @**************************************:
 */

#include <stdio.h>
#include <string.h>

char file[][1024] = {
#include "test.h"
};

int main(int argc, char const *argv[])
{
    printf("%s\n", file[0]);
    return 0;
}
