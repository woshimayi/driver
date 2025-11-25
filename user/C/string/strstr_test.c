/*
 * @*************************************:
 * @FilePath: /user/C/string/strstr_test.c
 * @version:
 * @Author: dof
 * @Date: 2023-07-10 09:42:08
 * @LastEditors: dof
 * @LastEditTime: 2024-03-20 14:03:33
 * @Descripttion:
 * @**************************************:
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static void cgi_get_info_from_file(const char *name, char *buf, unsigned int len)
{
	FILE *fp;
	unsigned int ret;

	fp = fopen(name, "r");
	if (fp == NULL)
	{
		printf("%s not exit.\r\n", name);
		return;
	}

	// ret = fread(buf, 1, len, fp);
	// if (ret >= len || ret == 0)
	// {
	// 	printf("read %s error, ret = %d.\r\n", name, ret);
	// }
	while (fgets(buf, len, fp))
	{
		// printf("buf = %s\n", buf);
		if (strstr(buf, "release time"))
		{
			printf("buf = %s\n", buf);
			break;
		}
	}
	buf[len - 1] = 0;
	fclose(fp);
}

#define PP(fmt, args...) printf("[mdm :%s(%d)] " fmt "\r\n", __func__, __LINE__, ##args)

#ifndef PP(fmt, args...)
#if 0
#define PP(fmt, args...) printf("[mdm :%s(%d)] " fmt "\r\n", __func__, __LINE__, ##args)
#else
#define PP(fmt, args...)
#endif
#endif

// int main(int argc, char const *argv[])
// {
// char deviceinfo[1024] = {0};
// char *release_time;
// char buf[256] = {0};

// cgi_get_info_from_file("/home/zs/Documents/driver/user/C/string/hi_version", deviceinfo, sizeof(deviceinfo));
// printf("deviceinfo = %s\n", deviceinfo);
// release_time = strstr(deviceinfo, "release time");

// if (release_time == NULL || (release_time = strstr(release_time, ":")) == NULL)
// {
// 	deviceinfo[0] = 0;
// 	release_time = deviceinfo;
// }
// else
// {
// 	release_time++;
// 	if (strlen(release_time))
// 	{
// 		release_time[strlen(release_time) - 1] = 0;
// 	}
// }

// printf("release time = %s\n", release_time);

// #define isspace0(c)	((c) == ' ')
// char *separator = NULL;
// char *str = "InternetGatewayDevice.WANDevice.1.WANConnectionDevice.1.WANPPPConnection.4.Username";
// char str1[256] = {0};
// char dnsSecondary[64] = {0};
// sscanf("2010::ca6c:20ff:fec4:ee9d/64", "%s/%*d", str);

// printf("1 str = %s--\n", strstr(str, "Username"));
// printf("2 str = %s--\n", strtok(str, "."));
// char *ret = strrchr(str, '.');
// snprintf(str1, ret - str + 2, "%s", str);
// PP("%p", str);
// PP("%p", ret);
// printf("3 str = %s-- \v%s\n", ret, str1);
// PP("str1 = %s", str1);

// separator = strtok(str, ",");

// if (separator != NULL)
// {
// 	/* break the string into two strings */
// 	// *separator = 0;
// 	 while ((isspace0(*separator)) && (*separator != 0))
// 	 {
// 		/* skip white space after comma */
// 		separator++;
// 	 }
// 	// separator++;

// 	strcpy(dnsSecondary, separator);
// 	printf("dnsSecondary=%s--\n", dnsSecondary);
// }

// printf("0x%x\n",  0x11111111 & ~0x10000000);

// char *s = NULL;
// if (s && 0 == strcasecmp(s, "FALse"))
// {
// 	char  t_v[8] = {0};
// 	void **v = &t_v[0];
// 	*v = (void *)1;
// 	printf("sss v = %d\n", (int *)(*v));
// }
// char *str_1 = "InternetGatewayDevice.LANDevice.1.Hosts.Host.4.";

// int lastNum;
// if (sscanf(str_1, "%*[^0-9]%d.", &lastNum) == 1) {
// 	printf("Last number: %d\n", lastNum);
// }

// return 0;
// }

#include <stdio.h>	// For printf, sscanf
#include <string.h> // For strlen
#include <stdlib.h> // For atoi (though sscanf directly handles conversion)
#include <ctype.h>	// For isdigit

/**
 * @brief 从字符串中提取最后一个数字。
 * 例如："InternetGatewayDevice.LANDevice.1.Hosts.Host.1." 返回 1
 * "abc.123.xyz" 返回 123
 * "no_number_here" 返回 0
 *
 * @param str 要处理的输入字符串。
 * @return 提取到的最后一个整数。如果没有找到数字，返回0。
 */
int extract_last_number(const char *str)
{
	if (str == NULL || *str == '\0')
	{
		return 0; // 空字符串或空指针，返回0
	}

	size_t len = strlen(str);
	int last_digit_pos = -1;			  // 最后一个数字字符的索引
	int first_digit_of_last_num_pos = -1; // 最后一个数字串的第一个数字字符的索引

	// 1. 从字符串末尾向前查找最后一个数字串的起始和结束位置
	// 遍历顺序：从字符串末尾开始向前。
	for (int i = len - 1; i >= 0; --i)
	{
		if (isdigit(str[i]))
		{
			last_digit_pos = i; // 记录找到的第一个数字（即最后一个数字串的末尾）

			// 找到最后一个数字后，继续向前查找这个数字串的起始位置
			for (int j = i; j >= 0; --j)
			{
				if (isdigit(str[j]))
				{
					first_digit_of_last_num_pos = j; // 记录当前数字的索引
				}
				else
				{
					break; // 遇到非数字字符，表示数字串结束
				}
			}
			break; // 找到最后一个数字串的范围后，退出外层循环
		}
	}

	if (last_digit_pos == -1)
	{
		return 0; // 字符串中没有找到任何数字
	}

	// 2. 使用 sscanf 从找到的数字串起始位置开始解析整数
	// sscanf 会自动跳过非数字字符（如果有的话），并读取一个整数。
	// 由于我们已经确定了数字串的精确起始位置，这里会直接解析出我们想要的数字。
	int extracted_num = 0;
	// str + first_digit_of_last_num_pos 指向最后一个数字串的起始字符
	sscanf(str + first_digit_of_last_num_pos, "%d", &extracted_num);

	return extracted_num;
}

int main()
{
	const char *path1 = "InternetGatewayDevice.LANDevice.1.Hosts.Host.1.";
	const char *path2 = "Folder.Sub.123.File.456";
	const char *path3 = "NoNumbersHere";
	const char *path4 = "SingleDigit.8";
	const char *path5 = "EndsWithNoNumber.";
	const char *path6 = "EmptyString";
	const char *path7 = "OnlyNumber77";
	const char *path8 = "Mixed12Alpha34Num"; // 测试混合字符串

	printf("String: \"%s\" -> Last Number: %d\n", path1, extract_last_number(path1));
	printf("String: \"%s\" -> Last Number: %d\n", path2, extract_last_number(path2));
	printf("String: \"%s\" -> Last Number: %d\n", path3, extract_last_number(path3));
	printf("String: \"%s\" -> Last Number: %d\n", path4, extract_last_number(path4));
	printf("String: \"%s\" -> Last Number: %d\n", path5, extract_last_number(path5));
	printf("String: \"%s\" -> Last Number: %d\n", path6, extract_last_number(path6));
	printf("String: \"%s\" -> Last Number: %d\n", path7, extract_last_number(path7));
	printf("String: \"%s\" -> Last Number: %d\n", path8, extract_last_number(path8)); // 预期输出 34

	char path[256] = {0};
	snprintf(path, sizeof(path), "%s", path1);
	path[sizeof(path1)] = '\0';
	printf("String: \"%s\" -> Last Number: %s\n", path, strrchr(path, '.'));

	int num = 5, ret = 0;
	ret = 90;
	num = -ret;
	printf("num = %d ret = %d | %d|\n", num, ret, atoi("-80"));
	return 0;
}
