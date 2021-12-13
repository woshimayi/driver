/*
 * @*************************************:
 * @FilePath: /user/C/split.c
 * @version:
 * @Author: dof
 * @Date: 2021-10-20 19:33:39
 * @LastEditors: dof
 * @LastEditTime: 2021-12-06 17:18:06
 * @Descripttion: 指定分隔符分割字符串
 * @**************************************:
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

char **split(const char *source, char flag);

int main()
{
	char str1[] = " abs   mk  oi pp";
	char str2[] = "*hello, world*";
	char str3[] = "192.168.1.1/24,192.168.5.1/24,192.168.4.1/24,192.168.3.1/24,192.168.2.1/24";
	char **p1, **p2, **p3;
	p1 = split(str1, ' ');
	int i = 0;

	for (i = 0; p1[i] != NULL; i++)
		printf("p1[%d] = %s\n", i, p1[i]);
	putchar('\n');

	p2 = split(str2, '*');
	for (i = 0; p2[i] != NULL; i++)
		printf("p2[%d] = %s\n", i, p2[i]);

	p3 = split(str3, ',');
	for (i = 0; p3[i] != NULL; i++)
		printf("p3[%d] = %s\n", i, p3[i]);

	// 释放内存
	free(p3);
	free(p2);
	free(p1);
	return 0;
}

/**
 * @name:
 * @test: test font
 * @msg:
 * @param {char} *source   指定分隔符分割字符串
 * @param {char} flag      指定分隔符分割符
 * @return 一个指向包含char*指针的数组的指针，即char **pt
 */
char **split(const char *source, char flag)
{
	char **pt;
	int j, n = 0;
	int count = 1;
	int len = strlen(source);
	char tmp[len + 1];
	tmp[0] = '\0';

	int i = 0;
	for (i = 0; i < len; ++i)
	{
		if (source[i] == flag && source[i + 1] == '\0')
			continue;
		else if (source[i] == flag && source[i + 1] != flag)
			count++;
	}
	// 多分配一个char*，是为了设置结束标志
	pt = (char **)malloc((count + 1) * sizeof(char *));

	count = 0;
	for (i = 0; i < len; ++i)
	{
		if (i == len - 1 && source[i] != flag)
		{
			tmp[n++] = source[i];
			tmp[n] = '\0';  // 字符串末尾添加空字符
			j = strlen(tmp) + 1;
			pt[count] = (char *)malloc(j * sizeof(char));
			strcpy(pt[count++], tmp);
		}
		else if (source[i] == flag)
		{
			j = strlen(tmp);
			if (j != 0)
			{
				tmp[n] = '\0';  // 字符串末尾添加空字符
				pt[count] = (char *)malloc((j + 1) * sizeof(char));
				strcpy(pt[count++], tmp);
				// 重置tmp
				n = 0;
				tmp[0] = '\0';
			}
		}
		else
			tmp[n++] = source[i];
	}
	// 设置结束标志
	pt[count] = NULL;

	return pt;
}
