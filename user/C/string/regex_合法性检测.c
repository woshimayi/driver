/*
 * @*************************************: 
 * @FilePath: /user/C/string/regex_合法性检测.c
 * @version: 
 * @Author: dof
 * @Date: 2021-10-20 19:33:39
 * @LastEditors: dof
 * @LastEditTime: 2023-12-22 15:15:17
 * @Descripttion: 正则表达式  使用用例
 * @**************************************: 
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <regex.h>

int ereg(char *pattern, char *value)
{
#if 0
	int r, cflags = 0;
	regmatch_t pm[10];
	const size_t nmatch = 10;
	regex_t reg;

	r = regcomp(&reg, pattern, cflags);
	if (r == 0)
	{
		r = regexec(&reg, value, nmatch, pm, cflags);
		if (r == REG_NOMATCH)
		{
			printf("No match\n");
		}
		else if (r != 0)
		{
			char errorbuf[100];
			regerror(r, &reg, errorbuf, sizeof(errorbuf));
			printf("regexec() failed: %s\n", errorbuf);
			// regfree(&reg);
			// return 1;
		}
		else
		{
			printf("Matched at offset %d\n", pm[0].rm_so);
		}

	}

	regfree(&reg);

	return r;
#else
	regex_t regex;
	int ret = regcomp(&regex, pattern, REG_EXTENDED);
	if (ret != 0)
	{
		printf("regcomp() failed\n");
		return 1;
	}

	ret = regexec(&regex, value, 0, NULL, 0);
	if (ret == REG_NOMATCH)
	{
		printf("No match\n");
	}
	else if (ret != 0)
	{
		printf("regexec() failed\n");
		regfree(&regex);
		return 1;
	}
	else
	{
		printf("Match\n");
	}
	printf("ret= %d\n", ret);
	regfree(&regex);
	return ret;
#endif
}


int isValidMac(char *value)
{
	int r; //r=0:valid, else not valid
	char *reg = "^[0-9A-F]\\([0-9A-F]\\:[0-9A-F]\\)\\{5\\}[0-9A-F]$";
	r = ereg(reg, value);
	return r;
}

int isValidIp(char *value)
{
	int r; //r=0:valid, else not valid
	char *reg = "^[0-9]\\{1,3\\}\\.[0-9]\\{1,3\\}\\.[0-9]\\{1,3\\}\\.[0-9]\\{1,3\\}$";
	r = ereg(reg, value);
	return r;
}

int isValiTime(char *value)
{
	int r; //r=0:valid, else not valid
	char *reg = "([1|0?][0-9]|2[0-3]):([0-5][0-9])";
	r = ereg(reg, value);
	return r;
}

int isValiWeek(char *value)
{
	if (NULL == value)
	{
		return 1;
	}

	int r; //r=0:valid, else not valid
	char *reg = "^([1-7]|[,?])+$";
	r = ereg(reg, value);
	return r;
}

int isValiDNS(char *value)
{
	if (NULL == value)
	{
		return 1;
	}

	int r; //r=0:valid, else not valid
	char *reg = "^(%*s|[,?])+$";
	r = ereg(reg, value);
	return r;
}




int main()
{
	printf("[%s%d] %d\n", __FUNCTION__, __LINE__, isValidIp("192.168.1.1"));
	printf("[%s%d] %d\n", __FUNCTION__, __LINE__, isValidMac("00:22:33:44:55:66"));
	printf("[%s%d] %d\n", __FUNCTION__, __LINE__, isValiTime("23:59"));
	printf("[%s%d] %d\n", __FUNCTION__, __LINE__, isValiWeek("2,45"));
	printf("[%s%d] %d\n", __FUNCTION__, __LINE__, isValiDNS("3000::2,3000::5,"));
	printf("[%s%d] %d\n", __FUNCTION__, __LINE__, isValiWeek("1234567"));
	// printf("r = %d\n", r);
	return 0;
}


