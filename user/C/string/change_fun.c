/*
 * @*************************************: 
 * @FilePath: /user/C/string/change_fun.c
 * @version: 
 * @Author: dof
 * @Date: 2021-10-20 19:33:39
 * @LastEditors: dof
 * @LastEditTime: 2023-08-19 20:13:32
 * @Descripttion: 
 * @**************************************: 
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int test_test(char *fun, int line, int d)
{
	printf("[%s:%d]  %d\n", fun, line, d);
	test(d);
	return 0;
}

int test(int d)
{
	printf("1111111111 %d\n", d);
	return 0;
}

#define test(d) test_test(__FUNCTION__, __LINE__, d)

int main()
{
	int ret = 0;
	ret = test(5);
	return 0;
}
