/*
 * @*************************************:
 * @FilePath: /user/C/string/repeat.c
 * @version:
 * @Author: dof
 * @Date: 2022-03-10 18:51:57
 * @LastEditors: dof
 * @LastEditTime: 2023-12-22 15:05:09
 * @Descripttion: 删除重复字符串
 * @**************************************:
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <regex.h>

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
