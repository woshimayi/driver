/*
 * @*************************************:
 * @FilePath: /user/C/string/file_data.c
 * @version:
 * @Author: dof
 * @Date: 2022-03-04 13:11:08
 * @LastEditors: dof
 * @LastEditTime: 2022-03-04 13:11:11
 * @Descripttion:创建文件并填充固定数据
 * @**************************************:
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

//#define FILL_DATA_VALUE  0xff
#define FILL_DATA_VALUE 0x30 // char 0

int c2i(char ch)
{
	if (isdigit(ch))
		return ch - 48;

	if (ch < 'A' || (ch > 'F' && ch < 'a') || ch > 'z')
		return -1;

	if (isalpha(ch))
		return isupper(ch) ? ch - 55 : ch - 87;

	return -1;
}

int hex2dec(char *hex)
{
	int len;
	int num = 0;
	int temp;
	int bits;
	int i;
	char str[64] = {0};

	if (NULL == hex)
	{
		printf("input para error \n");
		return 0;
	}

	if (('0' == hex[0]) && (('X' == hex[1]) || ('x' == hex[1])))
	{
		strcpy(str, &hex[2]);
	}
	else
	{
		strcpy(str, hex);
	}

	printf("input num = %s \n", str);

	len = strlen(str);

	for (i = 0, temp = 0; i < len; i++, temp = 0)
	{
		temp = c2i(*(str + i));

		bits = (len - i - 1) * 4;
		temp = temp << bits;

		num = num | temp;
	}
	return num;
}

int main(int argc, char **argv)
{
	FILE *l_pFile = NULL;
	int l_s32Rest = 0;
	unsigned int l_WriteLen = 0;
	unsigned int l_FileLen = 0;
	unsigned char TempData[1024] = {FILL_DATA_VALUE};

	if (3 != argc)
	{
		printf("usage: %s FileName  FileLen \n ", argv[0]);
		printf("eg: %s ./Outfile.bin 0x400 \n ", argv[0]);
		return 0;
	};

	const char *l_pFileName = argv[1];
	if (NULL == l_pFileName)
	{
		printf("input file name is NULL \n");
		return -1;
	}

	if (('0' == argv[2][0]) && (('X' == argv[2][1]) || ('x' == argv[2][1])))
	{
		l_FileLen = hex2dec(argv[2]);
	}
	else
	{
		l_FileLen = atoi(argv[2]);
	}

	printf("Need To Write Data Len %d \n", l_FileLen);
	printf("Fill Data Vale = 0x%x \n", FILL_DATA_VALUE);

	for (int i = 0; i < 1024; i++)
	{
		TempData[i] = FILL_DATA_VALUE;
	}

	l_pFile = fopen(l_pFileName, "w+");
	if (l_pFile == NULL)
	{
		printf("open file %s error \n", l_pFileName);
		return -1;
	}

	while (l_WriteLen < l_FileLen)
	{
		if (l_FileLen < 1024)
		{
			l_s32Rest = fwrite(TempData, 1, l_FileLen, l_pFile);
		}
		else
		{
			l_s32Rest = fwrite(TempData, 1, 1024, l_pFile);
		}

		if (l_s32Rest <= 0)
		{
			break;
		};

		l_WriteLen += l_s32Rest;
	}

	if (NULL != l_pFile)
	{
		fclose(l_pFile);
		l_pFile = NULL;
	}

	return 0;
}