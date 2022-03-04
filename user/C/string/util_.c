/*
 * @*************************************:
 * @FilePath: /user/C/string/util_.c
 * @version:
 * @Author: dof
 * @Date: 2022-03-04 13:08:44
 * @LastEditors: dof
 * @LastEditTime: 2022-03-04 13:10:38
 * @Descripttion:  字符转换
 * @**************************************:
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

int c2i(char ch)
{
	// 如果是数字，则用数字的ASCII码减去48, 如果ch = '2' ,则 '2' - 48 = 2
	if (isdigit(ch))
		return ch - 48;

	// 如果是字母，但不是A~F,a~f则返回
	if (ch < 'A' || (ch > 'F' && ch < 'a') || ch > 'z')
		return -1;

	// 如果是大写字母，则用数字的ASCII码减去55, 如果ch = 'A' ,则 'A' - 55 = 10
	// 如果是小写字母，则用数字的ASCII码减去87, 如果ch = 'a' ,则 'a' - 87 = 10
	if (isalpha(ch))
		return isupper(ch) ? ch - 55 : ch - 87;

	return -1;
}

int hex2dec(char *hex)
{
	int len;
	int num = 0;
	int temp;
	int bits;
	int i;
	char str[64] = {0};

	if (NULL == hex)
	{
		printf("input para error \n");
		return 0;
	}

	if (('0' == hex[0]) && (('X' == hex[1]) || ('x' == hex[1])))
	{
		strcpy(str, &hex[2]);
	}
	else
	{
		strcpy(str, hex);
	}

	printf("input num = %s \n", str);

	// 此例中 str = "1de" 长度为3, hex是main函数传递的
	len = strlen(str);

	for (i = 0, temp = 0; i < len; i++, temp = 0)
	{
		// 第一次：i=0, *(str + i) = *(str + 0) = '1', 即temp = 1
		// 第二次：i=1, *(str + i) = *(str + 1) = 'd', 即temp = 13
		// 第三次：i=2, *(str + i) = *(str + 2) = 'd', 即temp = 14
		temp = c2i(*(str + i));
		// 总共3位，一个16进制位用 4 bit保存
		// 第一次：'1'为最高位，所以temp左移 (len - i -1) * 4 = 2 * 4 = 8 位
		// 第二次：'d'为次高位，所以temp左移 (len - i -1) * 4 = 1 * 4 = 4 位
		// 第三次：'e'为最低位，所以temp左移 (len - i -1) * 4 = 0 * 4 = 0 位
		bits = (len - i - 1) * 4;
		temp = temp << bits;

		// 此处也可以用 num += temp;进行累加
		num = num | temp;
	}

	// 返回结果
	return num;
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

	l_s32Ret = hex2dec(argv[1]);
	printf("value hex = 0x%x \n", l_s32Ret);
	printf("value dec = %d \n", l_s32Ret);
	return 0;
}