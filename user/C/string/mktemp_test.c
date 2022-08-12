/*
 * @*************************************: 
 * @FilePath: /user/C/string/mktemp_test.c
 * @version: 
 * @Author: dof
 * @Date: 2022-07-20 15:52:18
 * @LastEditors: dof
 * @LastEditTime: 2022-07-20 16:02:03
 * @Descripttion: 产生唯一的文件
 * @**************************************: 
 */


#include <stdlib.h>

int main()
{
    char template[] = "./template-XXXXXX";
    mktemp(template);
    printf("template=%s\n", template);
}