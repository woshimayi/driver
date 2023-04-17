/*
 * @*************************************: 
 * @FilePath: /user/C/string/readlink.c
 * @version: 
 * @Author: dof
 * @Date: 2021-10-20 19:33:39
 * @LastEditors: dof
 * @LastEditTime: 2023-03-17 09:50:50
 * @Descripttion: 获取自身全路径
 * @**************************************: 
 */
#include <stdio.h>

#include <unistd.h>

int get_exe_path(char *buf, int count)
{

	return 0;
}


int main(int argc, char **argv)
{
	char path[1024];
	int rslt = readlink("/proc/self/exe", path, 1023);
	if (rslt < 0 || (rslt >= 1023))
	{
		return 0;
	}
	path[rslt] = '\0';
	printf("path = %s\n", path);

	return 0;
}
