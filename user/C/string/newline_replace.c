/*
 * @*************************************: 
 * @FilePath: /user/C/string/newline_replace.c
 * @version: 
 * @Author: dof
 * @Date: 2022-07-15 16:42:45
 * @LastEditors: dof
 * @LastEditTime: 2022-07-15 17:21:42
 * @Descripttion: 
 * @**************************************: 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define FILE_WEB_MANAGMENT_IP   "web_managment_ip"

int main(int argc, char const *argv[])
{
		char line[64] = {0};
		char webIp[64] = {0};
		FILE * fd = NULL;
		fd = fopen(FILE_WEB_MANAGMENT_IP, "r");
		if (NULL == fd)
		{
			perror("open fail");
		}
		if(fgets(line, sizeof(line), fd))
		{
			sscanf(line, "%s", webIp);
		}
		fclose(fd);
		printf("zzzzz ip = %s\n", webIp);

	return 0;
}
