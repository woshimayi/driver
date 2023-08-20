/*
 * @*************************************: 
 * @FilePath: /user/C/string/err.c
 * @version: 
 * @Author: dof
 * @Date: 2021-10-20 19:33:39
 * @LastEditors: dof
 * @LastEditTime: 2023-08-19 20:01:00
 * @Descripttion: 所有的错误码含义
 * @**************************************: 
 */
#include <errno.h>
#include <string.h>
#include <stdio.h>

int main()
{
	int i;
	for (i = 0; i < 140; ++i)
	{
		errno = i;
		printf("errno %d :\t\t%s\n", i, strerror(errno));
	}
	return 0;
}
