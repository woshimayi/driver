/*
 * @*************************************: 
 * @FilePath: /user/C/string/H1_3.c
 * @version: 
 * @Author: dof
 * @Date: 2022-07-20 17:02:15
 * @LastEditors: dof
 * @LastEditTime: 2022-07-20 17:30:27
 * @Descripttion: 去除两端的空格，使用api完成
 * @**************************************: 
 */



#include <stdio.h>
#include <string.h>
#include <stdlib.h>



char * strip(const char * str)
{
	char *temp = str;
	char *ret = str;

	int len = strlen(str);
	if (!len)
	{
		return NULL;
	}

	while (*temp)
	{
		if (*temp == ' ')
		{
			*temp++;
		}
		else
		{
			break;
		}
	}

	len = strlen(temp);
	if (!len)
	{
		return NULL;
	}

	temp += len-1;
	while (*temp)
	{
		if (*temp == ' ')
		{
			*temp = '\0';
		}
		else
		{
			break;
		}
		len--;
		*temp--;
	}

	ret = temp - len + 1;

	return ret;
}

int main(int argc, char const *argv[])
{
	char str[5000] = "     asdas asda sd asd asd asd asdasdaq qwqweq qdfg werwer asdsss   ";
	char *str_p = str;

	int count = 0;

	str_p = strip(str);
	printf("|%s|\n", str_p);
	printf("%d %d\n", strlen(str), strlen(str_p));

	return 0;
}

