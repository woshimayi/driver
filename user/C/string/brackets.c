/*
 * @*************************************:
 * @FilePath: /user/C/string/有效的括号.c
 * @version:
 * @Author: dof
 * @Date: 2021-10-20 19:33:39
 * @LastEditors: dof
 * @LastEditTime: 2021-12-07 14:05:59
 * @Descripttion: 判断字符串中的括号是否成对出现
 * @**************************************:
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


char pairs(char a)
{
	if (a == '}')
		return '{';
	if (a == ']')
		return '[';
	if (a == ')')
		return '(';
	return 0;
}

int isValid(char *s)
{
	int n = strlen(s);
	if (n % 2 == 1)
	{
		return 0;
	}
	int stk[n + 1], top = 0;
	int i = 0;
	for (i = 0; i < n; i++)
	{
		char ch = pairs(s[i]);
		if (ch)
		{
			if (top == 0 || stk[top - 1] != ch)
			{
				return 0;
			}
			top--;
		}
		else
		{
			stk[top++] = s[i];
		}
	}
	return top == 0;
}

int main(int argc, const char *argv[])
{
	char *str = "{[([[{{}}]])]}";
	if (isValid(str))
	{
		printf("ddddddddddd\n");
	}
	printf("fffffffffffffffff\n");
	return 0;
}

