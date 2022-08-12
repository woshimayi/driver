/*
 * @*************************************:
 * @FilePath: /user/C/string/getdelim_.c
 * @version:
 * @Author: dof
 * @Date: 2022-07-07 14:12:48
 * @LastEditors: dof
 * @LastEditTime: 2022-07-07 14:15:00
 * @Descripttion:
 * @**************************************:
 */

#include <stdio.h>
#include <stdlib.h>

#define _GNU_SOURCE
#include <stdio.h>;
#include <string.h>;
#include <sys/types.h>;

int main(void)
{
	FILE *fp;
	char *line = NULL;
	size_t len = 0;
	ssize_t read;
	fp = fopen("/etc/passwd", "r");
	if (fp == NULL)
		exit(-1);
	//            while ((read = getline(&line, &len, fp)) != -1) {
	while ((read = getdelim(&line, &len, '\n', fp)) != -1)
	{
		printf("Retrieved line of length %zu :\n", read);
		printf("%s", line);
	}
	if (line)
		free(line);
	return 0;
}