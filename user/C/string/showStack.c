/*
 * @*************************************: 
 * @FilePath: \undefinedc:\Users\ZS-offic\Documents\C\showStack.c
 * @version: 
 * @Author: dof
 * @Date: 2021-10-18 18:06:12
 * @LastEditors: dof
 * @LastEditTime: 2021-10-18 18:12:09
 * @Descripttion: 
 * @**************************************: 
 */

/*
 * gcc -g -rdynamic showStack.c 
 * addr2line 0x4009b7 -e a.out -f -C -s
 *
 *
 * */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <execinfo.h>
#include <unistd.h>

void ShowStack()
{
    int i;
    void *buffer[1024];
    int n = backtrace(buffer, 1024);
    char **symbols = backtrace_symbols(buffer, 1024);
    for (i = 0; i < n; i++)
    {
        printf("%s\n", symbols[i]);
    }
}

void SIgSegvProc(int signal)
{
	printf("enter this is sig %d\n", signal);
    if (signal == SIGSEGV)
    {
        printf("Recvive SIGSEGV signal\n");
        printf("-------------call stack----------\n");
        ShowStack();
	exit(0);
    }
    else
    {
        printf("this is sig %d\n", signal);
    }
}

void RegSig()
{
    signal(SIGSEGV, SIgSegvProc);
}

void fun3()
{
    printf("this is fun3\n");
    *(char *)0 = 1;
}

void fun2()
{
    printf("this is fun2\n");
    fun3();
}

void fun1()
{
    printf("this is fun1\n");
    fun2();
}

int main(int argc, char const *argv[])
{
    RegSig();
    fun1();
    return 0;
}
