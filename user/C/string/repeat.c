/*
 * @*************************************:
 * @FilePath: /user/C/string/repeat.c
 * @version:
 * @Author: dof
 * @Date: 2022-03-10 18:51:57
 * @LastEditors: dof
 * @LastEditTime: 2022-03-11 17:31:13
 * @Descripttion: 删除重复字符串
 * @**************************************:
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <regex.h>

char *delete_repeat(char *str1)
{
	char *str = str1;
	int len = strlen(str);
	int i, j, counter;
	for (i = 1; i <= len / 2; ++i)
	{
		for (j = i, counter = 0; j < len; ++j)
		{
			if (str[j] == str[j - i])
			{
				counter++;
			}
			else
			{
				counter = 0;
			}
			if (counter == i)
			{
				counter = 0;
				memmove(str + j - i, str + j, (len - j) * sizeof(char));
				j -= i;
				len -= i;
			}
		}
		str[j] = 0;
		printf("%s\n", str);
	}
	return str;
}

char *delete_repeat_1(char *str1)
{
	int i;
	char ebuff[256];
	int ret;
	int cflags;
	regex_t reg;
	regmatch_t rm[5];
	char *part_str = NULL;
	cflags = REG_EXTENDED | REG_ICASE;

	char *test_str = str1;
	char *reg_str = "(\d{6}[,]?)+";

	ret = regcomp(&reg, reg_str, cflags);
	if (ret)
	{
		regerror(ret, &reg, ebuff, 256);
		printf("1 %s\n", ebuff);
		goto end;
	}

	ret = regexec(&reg, test_str, 5, rm, 0);
	if (ret)
	{
		regerror(ret, &reg, ebuff, 256);
		printf("2 %s\n", ebuff);
		goto end;
	}

	regerror(ret, &reg, ebuff, 256);
	printf("3 result is:\n%s\n\n", ebuff);

end:
	regfree(&reg);
}

char *delete_repeat_2(char *input, char *out, int outLen)
{
	char *str = input;
	int j, in = 0;
	char cmd[512] = {0};
	int i = 0;
	char *p[20];
	char *buf = input;
	char *outer_ptr = NULL;
	char *inner_ptr = NULL;
	while ((p[in] = strtok_r(buf, ",", &outer_ptr)) != NULL)
	{
		buf = p[in];
		while ((p[in] = strtok_r(buf, " ", &inner_ptr)) != NULL)
		{
			in++;
			buf = NULL;
		}
		buf = NULL;
	}
	for (j = 0; j < in; j++)
	{
		printf(">%s<\n", p[j]);
		if (!strstr(cmd, p[j]))
		{
			strcat(cmd, j?",":"");
			strcat(cmd, p[j]);
			printf("cmd = %s\n", cmd);
		}
	}
	printf("len = %d\n", outLen);
	strncpy(out, cmd, outLen);
	return out;
}

int main(int argc, char const *argv[])
{
	/* code */
	char str[1024] = "104001,104006,104058,104001,104058,104001,104058,104001,104058";
	char p[512] = {0};

	printf("str = %s\n", str);
	delete_repeat_2(str, p, sizeof(p));
	printf("str = %s p = %s\n", str, p);

	return 0;
}
