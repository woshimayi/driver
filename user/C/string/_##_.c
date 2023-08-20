/*
 * @*************************************: 
 * @FilePath: /user/C/string/_##_.c
 * @version: 
 * @Author: dof
 * @Date: 2022-06-16 14:05:29
 * @LastEditors: dof
 * @LastEditTime: 2023-08-19 20:04:44
 * @Descripttion:  ## 连接字符
 * @**************************************: 
 */
#include <stdio.h>

#define A 12
#define B 13

#define _LINE(AA, BB) AA##BB
#define LINE(AA, BB) _LINE(AA, BB)

int main()
{
	int n = LINE(A, B);
	printf("n = %d\n", n);
	return 0;
}
