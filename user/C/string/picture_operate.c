/*
 * @*************************************:
 * @FilePath: /user/C/string/picture_operate.c
 * @version:
 * @Author: dof
 * @Date: 2022-03-04 13:11:47
 * @LastEditors: dof
 * @LastEditTime: 2022-03-04 13:11:47
 * @Descripttion: 批量处理图片
 * @**************************************:
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#define START_READ_POSITION 128
#define PHOTO_START_TIME 83641
// l_s32PhotoTime = 92809;

int Cut_file(char *InputFile)
{
	FILE *l_pFileInput = NULL;
	FILE *l_pFileOutput = NULL;
	char l_ars8OutputName[128] = {0};
	unsigned char l_arru8TempData[1024] = {0};
	int l_s32Ret = 0;
	static unsigned int ls_u32Num = 0;

	if (NULL == InputFile)
	{
		goto ERROR;
	}

	// sprintf(l_ars8OutputName,"./outfile/_%s",&InputFile[8]);
	sprintf(l_ars8OutputName, "./outfile/00%d.jpg", ls_u32Num++);

	// printf("out file name %s \n",l_ars8OutputName);

	l_pFileInput = fopen(InputFile, "rb+");
	if (NULL == l_pFileInput)
	{
		printf("input file open error\n");
		goto ERROR;
	}

	l_pFileOutput = fopen(l_ars8OutputName, "w+");
	if (NULL == l_pFileOutput)
	{
		printf("out file open error\n");
		goto ERROR;
	}

	fseek(l_pFileInput, START_READ_POSITION, SEEK_SET);

	while (!feof(l_pFileInput))
	{
		l_s32Ret = fread(l_arru8TempData, 1, 1024, l_pFileInput);
		if (l_s32Ret < 0)
		{
			break;
		}

		l_s32Ret = fwrite(l_arru8TempData, 1, l_s32Ret, l_pFileOutput);
		if (l_s32Ret < 0)
		{
			break;
		}
	}

ERROR:
	if (NULL != l_pFileOutput)
	{
		fclose(l_pFileOutput);
		l_pFileOutput = NULL;
	};

	if (NULL != l_pFileInput)
		;
	{
		fclose(l_pFileInput);
		l_pFileInput = NULL;
	}
}

int main(void)
{
	char l_arrs8InputName[128] = {0};
	char l_s8PhotoChannel = 0;
	int l_s32PhotoTime = 0;

	l_s8PhotoChannel = 3;
	l_s32PhotoTime = PHOTO_START_TIME;

	/**从第一通道开始**/
	for (int j = 1; j < l_s8PhotoChannel; j++)
	{
		for (int i = l_s32PhotoTime; i < 235959; i++)
		{
			memset(l_arrs8InputName, 0, sizeof(l_arrs8InputName));
			sprintf(l_arrs8InputName, "./image/%dY%06d.jpg", j, i);

			if (0 == access(l_arrs8InputName, F_OK))
			{
				printf("%s\n", l_arrs8InputName);
				Cut_file(l_arrs8InputName);
			}
		}
	}
}