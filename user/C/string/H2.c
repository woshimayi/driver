/*
 * @*************************************:
 * @FilePath: /user/C/string/H2.c
 * @version:
 * @Author: dof
 * @Date: 2022-07-20 17:36:00
 * @LastEditors: dof
 * @LastEditTime: 2022-07-20 19:44:04
 * @Descripttion:  字符串中 字符的个数，不区分大小写
 * @**************************************:
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#define MAX(a,b) (a>=b?a:b)
#define MIN(a,b) (a<b?a:b)
#define CMP(a,b) (MAX(a,b) - MIN(a,b))

#define SUB(a,b) (a - b)

int main(int argc, char const *argv[])
{
	char str[5000] = "zsxcszxc ASDSDAS UINGB 23467345";

	char *str_p = str;
	char c = '3';
	unsigned int count = 0;

	while (*str_p)
	{
		printf("%c\t", *str_p);
		printf(" %d", CMP(*str_p, c));
		if (((*str_p >= 65) && (*str_p <= 90))
			|| ((*str_p >= 97) && (*str_p <= 122)))
		{
			if ((0 == CMP(*str_p, c))
				|| (32 == CMP(*str_p, c)))
			{
				printf("\t%c", *str_p);
				count++;
			}
		}
		else
		{	
			if (0 == CMP(*str_p, c))
			{
				printf("\t%c", *str_p);
				count++;
			}
		}
		printf("\n");

		*str_p++;
	}

	printf("%d\n", count);

	return 0;
}
