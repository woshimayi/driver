/*
 * @*************************************: 
 * @FilePath: /user/C/string/backtrace_symbols_fd.c
 * @version: 
 * @Author: dof
 * @Date: 2021-10-21 09:02:37
 * @LastEditors: dof
 * @LastEditTime: 2021-12-07 14:07:54
 * @Descripttion: 打印堆栈信息
 * @**************************************: 
 */
#include <execinfo.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void myfunc3(void)
{
    int j, nptrs;
#define SIZE 100
    void *buffer[100];
    char **strings;

    nptrs = backtrace(buffer, SIZE);
    printf("backtrace() returned %d addresses\n", nptrs);

    /* The call backtrace_symbols_fd(buffer, nptrs, STDOUT_FILENO)
              would produce similar output to the following: */

    strings = backtrace_symbols(buffer, nptrs);
    if (strings == NULL)
    {
        perror("backtrace_symbols");
        exit(EXIT_FAILURE);
    }

    for (j = 0; j < nptrs; j++)
        printf("zzzzz %s\n", strings[j]);

    free(strings);
}

static void /* "static" means don't export the symbol... */
myfunc2(void)
{
    myfunc3();
}

void myfunc(int ncalls)
{
    myfunc2();
}

int main(int argc, char *argv[])
{
    myfunc(77);
    exit(EXIT_SUCCESS);
}