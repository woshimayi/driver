/*
 * @*************************************: 
 * @FilePath: /user/C/string/valgrind_test.c
 * @version: 
 * @Author: dof
 * @Date: 2022-06-24 14:44:15
 * @LastEditors: dof
 * @LastEditTime: 2022-06-24 14:51:35
 * @Descripttion: 
 * @**************************************: 
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>

int main(void)
{
	int *a = (int *)malloc(sizeof(int) * 3);
	a[0] = 1;
	a[1] = 2;
	a[2] = 3;
	a[3] = 4;
	free(a);
	printf("%d\n", a[0]);  // invalid read
	a[0] = 1;  // invalid write
	free(a);  // free a two times
	return 0;
}
