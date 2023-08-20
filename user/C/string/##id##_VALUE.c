/*
 * @*************************************:
 * @FilePath: /user/C/string/##id##_VALUE.c
 * @version:
 * @Author: dof
 * @Date: 2021-10-20 19:33:39
 * @LastEditors: dof
 * @LastEditTime: 2023-08-19 20:07:00
 * @Descripttion: ## 连接符
 * @**************************************:
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define SF_GetFeature(id)  _##id##_


int main()
{
	printf("SF_GetFeature(3) %s\n", SF_GetFeature(4));

	return 0;
}


