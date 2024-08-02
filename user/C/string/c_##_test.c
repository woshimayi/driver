/*
 * @*************************************: 
 * @FilePath: /user/C/string/c_##_test.c
 * @version: 
 * @Author: dof
 * @Date: 2023-10-20 13:49:51
 * @LastEditors: dof
 * @LastEditTime: 2024-08-01 15:45:00
 * @Descripttion:  ## test
 * @**************************************: 
 */
#include "stdio.h"

#define TO_STR(s) #s
#define TO_STR_SPLIT(s) _##s
#define COMB(str1,str2) str1##str2            // 只能是提前定义好的变量和常量，不能是变量

int main(int argc, char *argv[])
{
    int UART0= 115200;
	int UART1= 115201;
	int UART2= 115202;
	int UART3= 115203;
	int UART4= 115204;
	int UART5= 115205;
	int UART6= 115206;
	int UART7= 115207;

	for (int i = 0; i < 8; i++)
	{
		printf("UART0=%d\n", COMB(UART, 1)); // 字符串合并为变量UART0
	}

    printf("%s\n", TO_STR(3.14));//将数字变成字符串

	char *_VOIP = "sssssssssssss";

	printf("%s\n", TO_STR_SPLIT(VOIP));

	return 0;
}