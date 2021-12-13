/*
 * @*************************************:
 * @FilePath: /driver/user/C/IP_PPP_temp.c
 * @version:
 * @Author: dof
 * @Date: 2021-07-13 11:00:32
 * @LastEditors: dof
 * @LastEditTime: 2021-07-27 17:23:58
 * @Descripttion:  结构体指针赋值, 有疑点
 * @**************************************:
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct abc
{
	char str[20];
} abc;

typedef struct cde
{
	char *str1;
} cde;

int main()
{
	abc abc = {"abc"};
	cde cde = {"cde"};
	void *dest = &abc;
	void *dest1 = &cde;

	printf(" %p %p\n", &abc, &cde);
	printf("dest->str = %s\n", dest);
	printf("dest->str1 = %p, %s\n", dest1, (char *)dest1);
	return 0;
}