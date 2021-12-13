/*
 * @*************************************:
 * @FilePath: /user/C/split.c
 * @version:
 * @Author: dof
 * @Date: 2021-10-20 19:33:39
 * @LastEditors: dof
 * @LastEditTime: 2021-12-06 17:18:06
 * @Descripttion: ָ���ָ����ָ��ַ���
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

	// �ͷ��ڴ�
	free(p3);
	free(p2);
	free(p1);
	return 0;
}

/**
 * @name:
 * @test: test font
 * @msg:
 * @param {char} *source   ָ���ָ����ָ��ַ���
 * @param {char} flag      ָ���ָ����ָ��
 * @return һ��ָ�����char*ָ��������ָ�룬��char **pt
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
	// �����һ��char*����Ϊ�����ý�����־
	pt = (char **)malloc((count + 1) * sizeof(char *));

	count = 0;
	for (i = 0; i < len; ++i)
	{
		if (i == len - 1 && source[i] != flag)
		{
			tmp[n++] = source[i];
			tmp[n] = '\0';  // �ַ���ĩβ��ӿ��ַ�
			j = strlen(tmp) + 1;
			pt[count] = (char *)malloc(j * sizeof(char));
			strcpy(pt[count++], tmp);
		}
		else if (source[i] == flag)
		{
			j = strlen(tmp);
			if (j != 0)
			{
				tmp[n] = '\0';  // �ַ���ĩβ��ӿ��ַ�
				pt[count] = (char *)malloc((j + 1) * sizeof(char));
				strcpy(pt[count++], tmp);
				// ����tmp
				n = 0;
				tmp[0] = '\0';
			}
		}
		else
			tmp[n++] = source[i];
	}
	// ���ý�����־
	pt[count] = NULL;

	return pt;
}
