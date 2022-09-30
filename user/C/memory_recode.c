/*
 * @*************************************: 
 * @FilePath: /user/C/memory_recode.c
 * @version: 
 * @Author: dof
 * @Date: 2022-09-29 17:59:19
 * @LastEditors: dof
 * @LastEditTime: 2022-09-29 18:20:58
 * @Descripttion: linux 运行程序实时记录内存变换
 * @**************************************: 
 */


// read_statm.c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
	FILE *fp;
	char FILE_NAME[50] = {0};
	char data_buf[50] = {0};

	sprintf(FILE_NAME, "/proc/%d/statm", atoi(argv[1]));
	
	printf("VIRT\tRES\tSHR\tCODE\tLRS\tDATA\tDIRTY\n");
	fp = fopen(FILE_NAME, "r");
	while (fgets(data_buf, 50, fp))
	{
		printf("%s\n", data_buf);
		fseek(fp, 0, SEEK_SET);
		if (access(FILE_NAME, F_OK))
		{
			break;
		}
	}

	exit(0);
}