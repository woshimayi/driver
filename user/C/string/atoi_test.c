/*
 * @*************************************: 
 * @FilePath: /user/C/string/atoi_test.c
 * @version: 
 * @Author: dof
 * @Date: 2023-08-30 10:38:16
 * @LastEditors: dof
 * @LastEditTime: 2023-08-30 16:36:48
 * @Descripttion: atoi bug 逻辑 获取字符串的第一个数字
 * @**************************************: 
 */


#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char const *argv[])
{
	char *str = "1 Vendor Configuration File";
	char *str2 = "134Vendor Configuration File";
	printf("%d\n", atoi(str));
	printf("%d\n", atoi(str2));
	return 0;
}