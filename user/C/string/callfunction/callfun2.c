/*
 * @*************************************:
 * @FilePath: /user/C/string/callfun2.c
 * @version:
 * @Author: dof
 * @Date: 2021-10-20 19:33:39
 * @LastEditors: dof
 * @LastEditTime: 2021-12-07 14:13:40
 * @Descripttion: 函数指针示例
 * @**************************************:
 */
#include <stdlib.h>
#include <stdio.h>

// 回调函数
void populate_array(int *array, size_t arraySize, int (*getNextValue)(void))
{
	int i;
	for (i = 0; i < arraySize; i++)
	{
		array[i] = getNextValue();
		printf("111111\n");
	}
}

// 获取随机值
int getNextRandomValue(void)
{
	//	printf("2222\n");
	return rand();
}

int main(void)
{
	int myarray[10];
	int i;
	populate_array(myarray, 10, getNextRandomValue);
	for (i = 0; i < 10; i++)
	{
		printf("%d\n", myarray[i]);
	}
	printf("\n");
	return 0;
}
