/*
 * @*************************************:
 * @FilePath: /user/C/string/str2int.c
 * @version:
 * @Author: dof
 * @Date: 2022-03-04 13:10:27
 * @LastEditors: dof
 * @LastEditTime: 2022-03-04 13:10:27
 * @Descripttion:
 * @**************************************:
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

int String2int(char *strChar)
{
	int len = 0;
	const char *pstrCmp1 = "0123456789ABCDEF";
	const char *pstrCmp2 = "0123456789abcdef";

	char *pstr = NULL;
	int uiValue = 0;
	int j = 0;
	unsigned int t = 0;
	int i = 0;
	if (NULL == strChar)
		return -1;
	if (0 >= (len = strlen((const char *)strChar)))
		return -1;
	if (NULL != (pstr = strstr(strChar, "0x")) || NULL != (pstr = strstr(strChar, "0X")))
	{
		pstr = (char *)strChar + 2;

		if (0 >= (len = strlen((const char *)pstr)))
			return -1;
		for (i = (len - 1); i >= 0; i--)
		{
			if (pstr[i] > 'F')
			{
				for (t = 0; t < strlen((const char *)pstrCmp2); t++)
				{
					if (pstrCmp2[t] == pstr[i])
						uiValue |= (t << (j++ * 4));
				}
			}
			else
			{
				for (t = 0; t < strlen((const char *)pstrCmp1); t++)
				{
					if (pstrCmp1[t] == pstr[i])
						uiValue |= (t << (j++ * 4));
				}
			}
		}
	}
	else
	{
		uiValue = atoi((const char *)strChar);
	}
	return uiValue;
}

int main(int argc, char **argv)
{
	int l_s32Ret = 0;

	if (2 != argc)
	{
		printf("=====ERROR!======\n");
		printf("usage: %s Num \n", argv[0]);
		printf("eg 1: %s 0x400\n", argv[0]);
		return 0;
	}
	l_s32Ret = String2int(argv[1]);
	printf("value hex = 0x%x \n", l_s32Ret);
	printf("value dec = %d \n", l_s32Ret);
	return 0;
}