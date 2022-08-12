/*
 * @*************************************: 
 * @FilePath: /user/C/string/H2.c
 * @version: 
 * @Author: dof
 * @Date: 2022-07-19 19:43:45
 * @LastEditors: dof
 * @LastEditTime: 2022-07-20 16:41:37
 * @Descripttion:  字符串左后一个单词的长度 字符串末尾部位空格
 * @**************************************: 
 */


#include <stdio.h>
#include <string.h>



int main(int argc, char const *argv[])
{
	char str[5000] = "asdas asda sd asd asd asd asdasdaq qwqweq qdfg werwer asdsss   \n";
	char *str_p = str;

	int count = 0;

	int str_len = strlen(str) - 1;

	if (str_len <= 0)
	{
		printf("len fail\n");
		return 0;
	}

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

