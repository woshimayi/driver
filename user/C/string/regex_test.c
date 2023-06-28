/*
 * @*************************************:
 * @FilePath: /user/C/string/regex_test.c
 * @version:
 * @Author: dof
 * @Date: 2023-06-28 11:58:33
 * @LastEditors: dof
 * @LastEditTime: 2023-06-28 13:39:18
 * @Descripttion:
 * @**************************************:
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <regex.h>

int main()
{
	const char *pattern = "^[0-7,]+$";
	const char *subject = "1,2,3,4,5,6,7";

	regex_t regex;
	int ret = regcomp(&regex, pattern, REG_EXTENDED);
	if (ret != 0)
	{
		printf("regcomp() failed\n");
		return 1;
	}

	ret = regexec(&regex, subject, 0, NULL, 0);
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
	return 0;
}