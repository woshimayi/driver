/*
 * @*************************************: 
 * @FilePath: /user/C/string/double_struct.cpp
 * @version: 
 * @Author: dof
 * @Date: 2021-10-20 19:33:39
 * @LastEditors: dof
 * @LastEditTime: 2023-08-19 20:00:31
 * @Descripttion: 字符串拆解
 * @**************************************: 
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef unsigned char UINT8;
typedef char UINT16;

typedef struct
{
	UINT8 Index;
	UINT16 Maxsta;
} _OamSvaSsid;

typedef struct
{
	UINT8 Item;
	_OamSvaSsid SsidMaxsta;
} _OamSvaMaxUserConnect;

int main()
{
	_OamSvaMaxUserConnect oamSvaMaxUserConnect = {3, {2, 32}};

	char *endptr = NULL;
	char *str = "wl0.2";
	char str1[12] = "";

	strcpy(str1, str);

	printf("str1  =%s\n", str1);
	strtok(str1, ".");
	endptr = strtok(NULL, ".");

	printf("endptr = %s %d\n\n", endptr, atoi(endptr));

	return 0;
}