/*
 * @*************************************:
 * @FilePath: /user/C/string/args_test.c
 * @version:
 * @Author: dof
 * @Date: 2023-12-12 19:15:09
 * @LastEditors: dof
 * @LastEditTime: 2023-12-12 19:41:37
 * @Descripttion: 可变参数表介绍
 * @**************************************:
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

/**
 * @brief        求n个数中的最大值
 * @details
 * @param[in]     num 整数个数
 * @param[out]    ... 整数
 * @retval        最大整数
 * @par
 */
int max(int num, ...)
{
	int m = -0x7FFFFFFF; /* 32 系统中最小的整数 */
	va_list ap;
	va_start(ap, num);
	for (int i = 0; i < num; i++)
	{
		int t = va_arg(ap, int);
		if (t > m)
		{
			m = t;
		}
	}
	va_end(ap);
	return m;
}

int main(int argc, char *argv[])
{
	int n = max(5, 5, 6, 3, 8, 5); /* 求5 个整数中的最大值 */
	printf("n = %d\n", n);		   // NOLINT
	return 0;
}