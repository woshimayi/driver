/*
 * @*************************************:
 * @FilePath: /user/C/string/run_progress.c
 * @version:
 * @Author: dof
 * @Date: 2022-01-11 13:26:47
 * @LastEditors: dof
 * @LastEditTime: 2022-01-12 19:31:52
 * @Descripttion: 检测运行程序执行状态
 * @**************************************:
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define TRUE 1
#define FALSE 0

int isprocessPid(const char *process)
{
	char line[128] = {0};
	FILE *fpin = NULL;
	int flag = FALSE;
	unsigned int progessPid = -1;
	char cmd[128] = {0};

	if (NULL == cmd)
	{
		return -1;
	}

	snprintf(cmd, sizeof(cmd), "ps | pgrep %s", process);
	if ((fpin = popen(cmd, "r")) == NULL)
		perror("popen error");

	if (fgets(line, 128, fpin) != NULL) /* read from pipe */
	{
		return -1;
	}

	printf("line = %d\n", atoi(line));
	progessPid = atoi(line);

	if (progessPid)
	{
		flag = TRUE;
		printf("process exit\n");
	}
	return flag;
}

int main(int argc, char const *argv[])
{
	// isprocessPid("ping");

	double f = 0.534;
	int d = 2;
	printf("%d\n", (int)((f/d<1)?1:(f/d)));

	return 0;
}
