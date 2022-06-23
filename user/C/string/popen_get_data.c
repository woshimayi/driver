/*
 * @*************************************: 
 * @FilePath: /user/C/string/popen_get_data.c
 * @version: 
 * @Author: dof
 * @Date: 2022-06-23 12:00:36
 * @LastEditors: dof
 * @LastEditTime: 2022-06-23 13:01:09
 * @Descripttion: 
 * @**************************************: 
 */
#define _LINE_LENGTH 128


#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char const *argv[])
{
	/* code */
	FILE *file;
	char line[_LINE_LENGTH];
	file = popen("date +%", "r");
	if (NULL != file)
	{
		while (fgets(line, _LINE_LENGTH, file) != NULL)
		{
			printf("line=%s\n", line);
		}
	}
	else
	{
		return 1;
	}
	pclose(file);
	
	return 0;
}

