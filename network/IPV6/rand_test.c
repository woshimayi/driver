/*
 * @*************************************: 
 * @FilePath: /network/IPV6/rand_test.c
 * @version: 
 * @Author: dof
 * @Date: 2022-08-08 11:46:56
 * @LastEditors: dof
 * @LastEditTime: 2022-08-08 13:02:07
 * @Descripttion: 随机数
 * @**************************************: 
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>


int main(int argc, char const *argv[])
{
	while (1)
	{
		srand((unsigned)time(NULL));
		unsigned int data = rand() % 65535;
		if (data > 65535)
		{
			printf("%d\n", data);
			data -= 1024;
		}
		else if (data < 1024)
		{
			printf("%d\n", data);
			data += 1024;
		}
	}
	return 0;
}
