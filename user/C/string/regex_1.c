/*
 * @*************************************:
 * @FilePath: /user/C/regex_1.c
 * @version:
 * @Author: dof
 * @Date: 2022-09-08 16:23:07
 * @LastEditors: dof
 * @LastEditTime: 2022-09-08 16:23:12
 * @Descripttion:
 * @**************************************:
 */

#include <stdio.h>
#include <string.h>
#include <regex.h>

// 提取子串
char *getsubstr(char *s, regmatch_t *pmatch)
{
	static char buf[100] = {0};
	memset(buf, 0, sizeof(buf));
	memcpy(buf, s + pmatch->rm_so, pmatch->rm_eo - pmatch->rm_so);

	return buf;
}

int main(void)
{
	regmatch_t pmatch;
	regex_t reg;
	const char *pattern = "[a-z]+";			   // 正则表达式
	char buf[] = "HELLOsaiYear2012@gmail.com"; // 待搜索的字符串

	regcomp(&reg, pattern, REG_EXTENDED); //编译正则表达式
	int offset = 0;
	while (offset < strlen(buf))
	{
		int status = regexec(&reg, buf + offset, 1, &pmatch, 0);
		/* 匹配正则表达式，注意regexec()函数一次只能匹配一个，不能连续匹配，网上很多示例并没有说明这一点 */
		if (status == REG_NOMATCH)
			printf("No Match\n");
		else if (pmatch.rm_so != -1)
		{
			printf("Match:\n");
			char *p = getsubstr(buf + offset, &pmatch);
			printf("[%d, %d]: %s\n", offset + pmatch.rm_so + 1, offset + pmatch.rm_eo, p);
		}
		offset += pmatch.rm_eo;
	}
	regfree(&reg); //释放正则表达式

	return 0;
}