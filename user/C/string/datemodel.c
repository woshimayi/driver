/*
 * @*************************************: 
 * @FilePath: /user/C/string/datemodel.c
 * @version: 
 * @Author: dof
 * @Date: 2021-10-20 19:33:39
 * @LastEditors: dof
 * @LastEditTime: 2023-08-19 19:55:38
 * @Descripttion: sscan 截取字符串
 * @**************************************: 
 */


// Device.DHCPv4.Server.Pool.{i}.StaticAddress.{i}.Chaddr


#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(int args, char *argv[])
{
	char str[128] = "Device.DHCPv4.Server.Pool.1.StaticAddress.2.Chaddr";
	int i, j = 0;
	char str1[64] = {0};
	char str2[64] = {0};
	sscanf(str, "Device.DHCPv4.Server.Pool.%d.%s.%d.%s", &i, str1, &j, str2);
	printf("i = %d, str1 = %s, str2 = %s,  j = %d\n", i, str1, str2, j);
	return 0;
}