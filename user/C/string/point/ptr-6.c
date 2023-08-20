/*
 * @*************************************:
 * @FilePath: /user/C/string/ptr-6.c
 * @version:
 * @Author: dof
 * @Date: 2022-01-06 09:09:11
 * @LastEditors: dof
 * @LastEditTime: 2022-01-19 17:51:53
 * @Descripttion:
 * @**************************************:
 */

#include "stdio.h"

void fun(char **p)
{
	// p = (char *)malloc(16);
	// strncpy(p, "sss", 16);
	*p = strdup("aaaa");
	printf("p = %s\n", *p);
}




int main(int argc, char **argv)
{
	// unsigned int a = 10;
	// unsigned int *p = NULL;
	// p = &a;
	// printf("&a=%d\n",a);
	// printf("&a=%p\n",&a);
	// *p = 20;
	// printf("a=%d\n",a);

	// char *a = "asd";
	// char *p = NULL;
	// p = a;
	// printf("&a = %s\n", a);
	// printf("p = %s\n", p);
	// strcpy(a ,"dddd");
	// printf("a = %s\n", a);

	char a[120] = "sssssssssssssssaaaaaaaaaaaaa";
	char *p = a;
	printf("main 1 p = %s\n", p);
	fun(&p);
	printf("main p = %s\n", p);
	printf("main a = %s\n", a);
	free(p);

	return 0;
}