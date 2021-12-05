/*
 * @*************************************: 
 * @FilePath: \C\call_fun_print_funname.c
 * @version: 
 * @Author: dof
 * @Date: 2021-10-13 10:15:18
 * @LastEditors: dof
 * @LastEditTime: 2021-10-13 10:15:37
 * @Descripttion:  打印回调函数名    编译是如下  gcc func.c  -rdynamic
 * @**************************************: 
 */

#include <stdio.h>
#include <execinfo.h>

typedef void (*eventHandler)(int);

void test1(int a)
{
	printf("%s %d\n", __FUNCTION__, __LINE__);
	printf("%d\n", a);
}

void test2(int b, eventHandler p)
{
	backtrace_symbols_fd((void *)&p, 1, 1);
	printf("%s %d\n", __FUNCTION__, __LINE__);
	printf("%d\n", b);
	p(b);
}

int main(int argc, char *argv[])
{
	//int (*p)(int a);
	//p = test2;
	//p(10);
	test2(2, test1);
	//backtrace_symbols_fd((void *)&p, 1, 1);
}
