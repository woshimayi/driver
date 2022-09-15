/*
 * @*************************************:
 * @FilePath: /user/C/string/popen_.c
 * @version:
 * @Author: dof
 * @Date: 2021-10-20 19:33:39
 * @LastEditors: dof
 * @LastEditTime: 2022-09-15 18:48:20
 * @Descripttion:
 * @**************************************:
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int searchPid(char * pidName)
{
	FILE *fd;
	char tmp[16] = {0};
	int cnt = 0;
	unsigned int pid = 0;
	char cmd[256] = {0};

	snprintf(cmd, sizeof(cmd), "pgrep %s", pidName);
	fd = popen(cmd, "r");
	if (!fd)
	{
		pclose(fd);
		return 0;
	}

	cnt = fread(tmp, sizeof(char), sizeof(tmp), fd);
	if (!cnt)
	{
		pclose(fd);
		return 0;
	}
	pid = atoi(tmp);
	// printf("tmp = %s pid = %d\n ", tmp, atoi(tmp));
	return pid;
}

int isPidRuning(int pid)
{
	FILE *fd;
	char tmp[16] = {0};
	int cnt = 0;
	unsigned int tmpPid = 0;
	char cmd[256] = {0};
	
	snprintf(cmd, sizeof(cmd), "pgrep pppd");
	fd = popen(cmd, "r");
	if (!fd)
	{
		pclose(fd);
		return 0;
	}

	while (fread(tmp, sizeof(char), sizeof(tmp), fd))
	{
		tmpPid = atoi(tmp);
		if (pid == tmpPid)
		{
			pclose(fd);
			return 1;
		}
		printf("tmp = %s pid = %d\n ", tmp, pid);
	}
	pclose(fd);
	return 0;
}



int main(int argc, char const *argv[])
{
	unsigned int pid = 0;
	pid = searchPid("pppd");
	printf("pid = %d\n", pid);
	printf("runing = %d\n", isPidRuning(pid));
	printf("runing = %d\n", isPidRuning(234));
	return 0;
}
