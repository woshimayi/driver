/*
 * @*************************************:
 * @FilePath: /user/C/string/util_strtol.c
 * @version:
 * @Author: dof
 * @Date: 2021-12-27 14:37:44
 * @LastEditors: dof
 * @LastEditTime: 2021-12-27 14:41:44
 * @Descripttion: 字符串 分割 整数 和剩余字符串
 * @**************************************:
 */

// #include <stdio.h>
// #include <stdlib.h>

// int main()
// {
//    char str[30] = "2030300 This is test";
//    char *ptr;
//    long ret;

//    ret = strtol(str, &ptr, 10);
//    printf("数字（无符号长整数）是 %ld\n", ret);
//    printf("字符串部分是 |%s|", ptr);

//    return(0);
// }

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int str2i(const char *str, char split, char **endptr)
{
	int ret = 0;
	ret = strtol(str, endptr, 10);
	if (*endptr != NULL)
	{
		if (**endptr == split)
		{
			*endptr += 1;
		}
	}

	return ret;
}

int main(void)
{
	char *test_str = "1,2,3,4,5";
	char *end = NULL;
	int res[5] = {0};
	char *p_shift = test_str;

	for (int i = 0; i < 5; i++)
	{
		res[i] = str2i(p_shift, ',', &end);
		p_shift = end;
	}

	for (int i = 0; i < 5; i++)
	{
		printf("%d ", res[i]);
	}
}