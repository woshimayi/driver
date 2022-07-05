/*
 * @*************************************:
 * @FilePath: /user/C/string/splites_test.c
 * @version:
 * @Author: dof
 * @Date: 2022-06-27 19:23:22
 * @LastEditors: dof
 * @LastEditTime: 2022-06-27 19:43:00
 * @Descripttion: 字符串分割
 * @**************************************:
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct
{
	char **str; // the PChar of string array
	size_t num; // the number of string
} IString;

/** \Split string by a char
 *
 * \param  src:the string that you want to split
 * \param  delim:split string by this char
 * \param  istr:a srtuct to save string-array's PChar and string's amount.
 * \return  whether or not to split string successfully
 *
 */
int Split(char *src, char *delim, IString *istr) // split buf
{
	int i;
	char *str = NULL, *p = NULL;

	(*istr).num = 1;
	str = (char *)calloc(strlen(src) + 1, sizeof(char));
	if (str == NULL)
	{
		return 0;
	}

	(*istr).str = (char **)calloc(1, sizeof(char *));
	if ((*istr).str == NULL)
	{
		return 0;
	}
	
	strcpy(str, src);

	p = strtok(str, delim);
	(*istr).str[0] = (char *)calloc(strlen(p) + 1, sizeof(char));
	if ((*istr).str[0] == NULL)
	{
		return 0;
	}

	strcpy((*istr).str[0], p);
	for (i = 1; p = strtok(NULL, delim); i++)
	{
		(*istr).num++;
		(*istr).str = (char **)realloc((*istr).str, (i + 1) * sizeof(char *));
		if ((*istr).str == NULL)
		{
			return 0;
		}
		(*istr).str[i] = (char *)calloc(strlen(p) + 1, sizeof(char));
		if ((*istr).str[0] == NULL)
		{
			return 0;
		}
		strcpy((*istr).str[i], p);
	}
	free(str);
	str = p = NULL;

	return 1;
}

int main()
{
	int i;
	char s[] = "data0|data1|data2|data3|data4|data5|data6|data7|data8";
	IString istr;

	if (Split(s, "|", &istr))
	{
		for (i = 0; i < istr.num; i++)
		{
			printf("%s\n", istr.str[i]);
		}

		// when you don't ues it,you must to free memory.
		for (i = 0; i < istr.num; i++)
		{
			free(istr.str[i]);
		}
		free(istr.str);
	}
	else
	{
		printf("memory allocation failure!\n");
	}

	system("pause");
	return 0;
}