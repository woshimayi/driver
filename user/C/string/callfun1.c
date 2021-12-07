/*
 * @*************************************: 
 * @FilePath: /user/C/string/callfun1.c
 * @version: 
 * @Author: dof
 * @Date: 2021-10-20 19:33:39
 * @LastEditors: dof
 * @LastEditTime: 2021-12-07 14:13:08
 * @Descripttion: 函数指针示例
 * @**************************************: 
 */
#include <stdio.h>

int add_ret(int a, int b)
{
    printf("11111111");
    return a + b ;
}

int add(int a, int b, int (*add_value)())
{
    (*add_value);
    return 0;
}

int main(void)
{
    int sum = add(3, 4, add_ret);
    printf("sum:%d\n", sum);
    printf("=\r");
    printf("===\r");
    printf("========\r");
    return 0 ;
}

