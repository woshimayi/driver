/*
 * @*************************************:
 * @FilePath: /user/C/regex_2.c
 * @version:
 * @Author: dof
 * @Date: 2022-09-08 16:24:09
 * @LastEditors: dof
 * @LastEditTime: 2022-09-08 16:36:04
 * @Descripttion:
 * @**************************************:
 */

#include <stdio.h>
#include <string.h>
#include <regex.h>


int isMathPathDot(char * path)
{
	regmatch_t pmatch;
	regex_t reg;
	const char *pattern = "[\.]$";			// 正则表达式
	int ret = 0;
	char buf[256] = {0};
	strncpy(buf, path, sizeof(buf));
	regcomp(&reg, pattern, REG_EXTENDED); //编译正则表达式

	int status = regexec(&reg, buf, 1, &pmatch, 0);
	/* 匹配正则表达式，注意regexec()函数一次只能匹配一个，不能连续匹配，网上很多示例并没有说明这一点 */
	if (status == REG_NOMATCH)
	{
		printf("No Match\n");
		ret = 1;
	}
	else
	{
		printf("Match:\n");
	}
	regfree(&reg); //释放正则表达式

	return ret;
}

int main(void)
{
	char buf[] = "InternetGatewayDevice.WANDevice.1.WANConnectionDevice.1.WANPPPConnection.3"; // 待搜索的字符串
	if (isMathPathDot(buf))
	{
		printf("zzzzzz Match");
	}
	else
	{
		printf("zzzzzz No Match");
	}

	return 0;
}