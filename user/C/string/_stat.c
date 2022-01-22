/*
 * @*************************************: 
 * @FilePath: /user/C/string/_stat.c
 * @version: 
 * @Author: dof
 * @Date: 2021-10-20 19:33:39
 * @LastEditors: dof
 * @LastEditTime: 2022-01-20 17:54:54
 * @Descripttion: 
 * @**************************************: 
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/stat.h>
#include <unistd.h>


int main()
{
	char cmd[128] = {0};
	int count = 0;

	struct stat buf;
	int ret = 0;
	if (0 != (ret = stat("./123", &buf)))
	{
		strerror(ret);
	}

	while (buf.st_size < 1000000)
	{
		if (0 != stat("./123", &buf))
		{
			strerror(ret);
		}
	}
	printf("/etc/passwd file size = %d\n", buf.st_size);

	FILE *fd = NULL;
	fd =  fopen("./123", "r");
	while (fread((void *)cmd, 1, 25, fd))
	{
		printf("%s", cmd);
	}

	return 0;
}

