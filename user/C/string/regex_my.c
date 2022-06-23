/*
 * @*************************************: 
 * @FilePath: /user/C/string/regex_my.c
 * @version: 
 * @Author: dof
 * @Date: 2021-11-30 11:39:26
 * @LastEditors: dof
 * @LastEditTime: 2022-03-10 19:25:59
 * @Descripttion: 
 * @**************************************: 
 */
#if 0
#include <sys/types.h>
#include <regex.h>
#include <stdio.h>

int main(int argc, char **argv)
{
	// if (argc != 3) {
	//     printf("Usage: %s RegexString Text\n", argv[0]);
	//     return 1;
	// }
	const char *pregexstr = "ldwsss@itcast.com";
	const char *ptext = "^[a-zA-Z0-9]+@[a-zA-Z0-9]+.[a-zA-Z0-9]+";
	regex_t oregex;
	int nerrcode = 0;
	char szerrmsg[1024] = {0};
	size_t unerrmsglen = 0;
	if ((nerrcode = regcomp(&oregex, pregexstr, REG_EXTENDED | REG_NOSUB)) == 0)
	{
		if ((nerrcode = regexec(&oregex, ptext, 0, NULL, 0)) == 0)
		{
			printf("%s matches %s\n", ptext, pregexstr);
			regfree(&oregex);
			return 0;
		}
	}
	unerrmsglen = regerror(nerrcode, &oregex, szerrmsg, sizeof(szerrmsg));
	unerrmsglen = unerrmsglen < sizeof(szerrmsg) ? unerrmsglen : sizeof(szerrmsg) - 1;
	szerrmsg[unerrmsglen] = '\0';
	printf("ErrMsg: %s\n", szerrmsg);
	regfree(&oregex);

	return 1;
}
#endif

#define _GNU_SOURCE
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <regex.h>
int main(void)
{
	int i;
	char ebuff[256];
	int ret;
	int cflags;
	regex_t reg;
	regmatch_t rm[5];
	char *part_str = NULL;
	cflags = REG_EXTENDED | REG_ICASE;
	// char *test_str = "Hello World";
	// char *reg_str = "H(.*)o";

	// char *test_str = "123-456-789";  // 获取456
	// char *reg_str = "[-](.*)[-]";

	char *test_str = "asdas=4";
	char *reg_str = "(\w)=(\d)";



	ret = regcomp(&reg, reg_str, cflags);
	if (ret)
	{
		regerror(ret, &reg, ebuff, 256);
		fprintf(stderr, "%s\n", ebuff);
		goto end;
	}

	ret = regexec(&reg, test_str, 5, rm, 0);
	if (ret)
	{
		regerror(ret, &reg, ebuff, 256);
		fprintf(stderr, "%s\n", ebuff);
		goto end;
	}

	regerror(ret, &reg, ebuff, 256);
	fprintf(stderr, "result is:\n%s\n\n", ebuff);

	for (i = 0; i < 5; i++)
	{
		if (rm[i].rm_so > -1)
		{
			part_str = strndup(test_str + rm[i].rm_so, rm[i].rm_eo - rm[i].rm_so);
			fprintf(stderr, "%s\n", part_str);
			free(part_str);
			part_str = NULL;
		}
	}
end:
	regfree(&reg);
	return 0;
}