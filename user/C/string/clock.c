/*
 * @*************************************: 
 * @FilePath: /user/C/string/clock.c
 * @version: 
 * @Author: dof
 * @Date: 2022-01-23 16:32:53
 * @LastEditors: dof
 * @LastEditTime: 2023-08-19 20:16:40
 * @Descripttion: 计算时间间隔
 * @**************************************: 
 */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
void test()
{
	int a = 9894;
	while (a--)
	{
	}
}
int main()
{
	clock_t begin, end;
	begin = clock();
	test();
	end = clock();
	printf("%lf\n", (double)(end - begin) / CLOCKS_PER_SEC);
	return 0;
}
