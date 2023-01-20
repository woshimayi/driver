/*
 * @*************************************:
 * @FilePath: /user/C/string/point_test.c
 * @version:
 * @Author: dof
 * @Date: 2022-12-08 13:45:15
 * @LastEditors: dof
 * @LastEditTime: 2022-12-09 10:28:52
 * @Descripttion:
 * @**************************************:
 */

#include <stdio.h>

void fun1(char *p)
{
	char *b = "eee";
	
	// 不改变原有参数值， 无法返回数据
	p = b;

	// 不改变原有参数值
	// strcpy(p, b);
}

void fun(char **p)
{
	char *b = "rrr";
	
	// 不改变原有参数值
	*p = b;

	// 改变原有参数值
	// strcpy(*p, b);
}

int main(int argc, char *argv[])
{
	// char *a = "www";
	char a[128] = "www";
	int *q = a;
	printf("%s\n", a);
	printf("%s\n", q);

	fun1(q);
	printf("%s\n", a);
	printf("%s\n", q);

	// 不改变原有参数值
	fun(&q);
	printf("%s\n", a);
	printf("%s\n", q);

	char mountcmd[128] = {0};
	snprintf(mountcmd, sizeof mountcmd, "mount -o remount");

		return 0;
}