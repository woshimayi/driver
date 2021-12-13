/*
 * @*************************************:
 * @FilePath: \user\C\str_shift.c
 * @version:
 * @Author: dof
 * @Date: 2021-08-20 16:49:53
 * @LastEditors: dof
 * @LastEditTime: 2021-08-20 17:11:16
 * @Descripttion:
 * @**************************************:
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int step(char *pStr, int step)
{
	int len = strlen(pStr);
	char temp[len + 1];

	printf("pStr = %s %d, %s %s\n", pStr, len, pStr + 2, pStr + len - step);
	strncpy(temp, pStr + len - step, step);
	strncpy(&temp[step], pStr, len);

	temp[len + 1] = '\0';

	return 0;
}

int main(int argc, char const *argv[])
{
	char str[] = "abcdefghijklmn";
	// step(str, 2);
	printf("str = %p\n", str);
	char *p = &str;
	// strncpy(str, "ssss", sizeof(str));
	printf("p = %s\n", p);
	return 0;
}
