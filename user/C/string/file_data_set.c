/*
 * @*************************************:
 * @FilePath: /user/C/string/file_data_set.c
 * @version:
 * @Author: dof
 * @Date: 2022-03-04 13:12:51
 * @LastEditors: dof
 * @LastEditTime: 2022-03-04 13:12:55
 * @Descripttion: 文件固定位置插入数据
 * @**************************************:
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BASIC_FILE_NAME "./nandflash.bin"
#define UBOOT_FILE_NAME "./u-boot.bin"
#define KERNEL_FILE_NAME "./kernel.bin"
#define ROOTFS_FILE_NAME "./rootfs.bin"
#define APP_FILE_NAME "./app.bin"

#define UBOOT_POSITION 0x00
#define KERNEL_POSITION 0x100000
#define ROOTFS_POSITION 0x500000
#define APP_POSITION 0x2700000

int InsertData(FILE *pfBasic, FILE *psInsert, int s32Position)
{
	int l_S32Ret = 0;
	unsigned char l_arru8Temp[1024] = {0xff};

	fseek(pfBasic, s32Position, SEEK_SET);
	fseek(psInsert, 0, SEEK_SET);
	while (1)
	{
		l_S32Ret = fread(l_arru8Temp, 1, 1024, psInsert);
		if (l_S32Ret > 0)
		{
			l_S32Ret = fwrite(l_arru8Temp, 1, l_S32Ret, pfBasic);
			if (l_S32Ret <= 0)
			{
				printf("line %d error l_S32Ret = %d \n", __LINE__, l_S32Ret);
				return -1;
			}
		}
		else
		{
			break;
		}
	}

	return 0;
}

int main(void)
{
	int l_s32Ret = 0;
	FILE *l_pfBasec = NULL;
	FILE *l_pfUboot = NULL;
	FILE *l_pfKernel = NULL;
	FILE *l_pfRootfs = NULL;
	FILE *l_pfApp = NULL;

	l_pfBasec = fopen(BASIC_FILE_NAME, "r+");
	if (NULL == l_pfBasec)
	{
		printf("line %d error \n", __LINE__);
		goto ERROR;
	}

	l_pfUboot = fopen(UBOOT_FILE_NAME, "r");
	if (NULL == l_pfUboot)
	{
		printf("line %d error \n", __LINE__);
		goto ERROR;
	}

	l_pfKernel = fopen(KERNEL_FILE_NAME, "r");
	if (NULL == l_pfKernel)
	{
		printf("line %d error \n", __LINE__);
		goto ERROR;
	}

	l_pfRootfs = fopen(ROOTFS_FILE_NAME, "r");
	if (NULL == l_pfRootfs)
	{
		printf("line %d error \n", __LINE__);
		goto ERROR;
	}

	l_pfApp = fopen(APP_FILE_NAME, "r");
	if (NULL == l_pfApp)
	{
		printf("line %d error \n", __LINE__);
		goto ERROR;
	}

	if (0 > InsertData(l_pfBasec, l_pfUboot, UBOOT_POSITION))
	{
		printf("line %d error \n", __LINE__);
		goto ERROR;
	}

	if (0 > InsertData(l_pfBasec, l_pfKernel, KERNEL_POSITION))
	{
		printf("line %d error \n", __LINE__);
		goto ERROR;
	}

	if (0 > InsertData(l_pfBasec, l_pfRootfs, ROOTFS_POSITION))
	{
		printf("line %d error \n", __LINE__);
		goto ERROR;
	}

	if (0 > InsertData(l_pfBasec, l_pfApp, APP_POSITION))
	{
		printf("line %d error \n", __LINE__);
		goto ERROR;
	}

ERROR:
	if (NULL != l_pfBasec)
	{
		fclose(l_pfBasec);
		l_pfBasec = NULL;
	}

	if (NULL != l_pfUboot)
	{
		fclose(l_pfUboot);
		l_pfUboot = NULL;
	}

	if (NULL != l_pfKernel)
	{
		fclose(l_pfKernel);
		l_pfKernel = NULL;
	}

	if (NULL != l_pfRootfs)
	{
		fclose(l_pfRootfs);
		l_pfRootfs = NULL;
	}

	if (NULL != l_pfApp)
	{
		fclose(l_pfApp);
		l_pfApp = NULL;
	}

	return 0;
}