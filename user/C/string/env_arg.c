/*
 * @*************************************: 
 * @FilePath: /user/C/string/env_arg.c
 * @version: 
 * @Author: dof
 * @Date: 2022-07-15 10:16:24
 * @LastEditors: dof
 * @LastEditTime: 2023-08-19 20:00:51
 * @Descripttion: 获取环境变量
 * @**************************************: 
 */

#include <stdlib.h>
#include <stdio.h>

int main(void)
{
	// setenv("IP", "192.168.1.100", 1);
	printf("PATH=%s\n", getenv("IP"));
	printf("PATH=%s\n", getenv("IP"));
	printf("PATH=%s\n", getenv("PATH"));
	return 0;
}
