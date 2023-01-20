/*
 * @*************************************: 
 * @FilePath: /user/C/string/if_mul_fun.c
 * @version: 
 * @Author: dof
 * @Date: 2022-12-01 16:47:38
 * @LastEditors: dof
 * @LastEditTime: 2022-12-05 18:52:19
 * @Descripttion: 
 * @**************************************: 
 */

#include <stdio.h>
#include <string.h>

int fun(char * fun, int line, char *str)
{
	printf("[%s:%d] %s\n", fun, line, str);
	return 0;
}

#define FUN(str) fun(__FUNCTION__, __LINE__, str)


int main(int argc, char const *argv[])
{
	if (FUN("1111"))
	{
		printf("1111");
	}
	else if (FUN("2222"))
	{
		printf("2222");
	}
	else if (FUN("3333"))
	{
		printf("3333");
	}
	return 0;
}
