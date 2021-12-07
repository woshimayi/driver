/*
 * @*************************************: 
 * @FilePath: /user/C/string/callfun.cpp
 * @version: 
 * @Author: dof
 * @Date: 2021-10-20 19:33:39
 * @LastEditors: dof
 * @LastEditTime: 2021-12-07 14:12:01
 * @Descripttion: 函数指针示例
 * @**************************************: 
 */
#include<stdio.h>
#include<string.h>
#include<stdlib.h>

#define PP printf("[%s:%d]\n", __FUNCTION__, __LINE__);

typedef void (*CALLFUN)(char *);

void CallPrintfText(CALLFUN fp, char *s)
{
    fp(s);
}

void PrintfText(char *s)
{
    printf(s);
}

int main()
{
    CallPrintfText(PrintfText, "Hello World\n");
    PP
    return 0;
}

