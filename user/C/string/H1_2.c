/*
 * @*************************************: 
 * @FilePath: /user/C/string/H1_2.c
 * @version: 
 * @Author: dof
 * @Date: 2022-07-20 16:46:27
 * @LastEditors: dof
 * @LastEditTime: 2022-07-20 17:01:22
 * @Descripttion: 去除字符串 两端的 的空格
 * @**************************************: 
 */


#include <stdio.h>
#include <string.h>
#include <stdlib.h>



int main(int argc, char const *argv[])
{
	char str[5000] = "     asdas asda sd asd asd asd asdasdaq qwqweq qdfg werwer asdsss   \n";
	char *str_p = str;

	int count = 0;

	int str_len = strlen(str) - 1;

	if (str_len <= 0)
	{
		printf("len fail\n");
		return 0;
	}

	// 去除前面的空格
	while (*str_p)
	{
		if (*str_p == ' ')
		{
			*str_p++;
		}
		else
		{
			break;
		}
	}

	printf("|%s|\n", str_p);

	str_p = str_p + str_len - 1;

	// strip 去除最后的空格
	while (*str_p)
	{
		if (*str_p == ' ')
		{
			*str_p = '\0';
		}
		else
		{
			break;
		}
		*str_p--;
	}



	printf("%s||||\n", str);

	for (int i = 0; i < str_len; i++)
	{
		printf("%c\n", *str_p);
		if (*str_p != ' ')
		{
			count++;
		}
		else
		{
			break;
		}
		str_p--;
	}

	printf("%d %s\n", count, str_p);
	return 0;

	return 0;
}