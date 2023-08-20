/*
 * @*************************************:
 * @FilePath: /user/C/string/get_path.c
 * @version:
 * @Author: dof
 * @Date: 2023-08-19 19:29:10
 * @LastEditors: dof
 * @LastEditTime: 2023-08-19 19:40:28
 * @Descripttion:  获取文件的绝对路径
 * @**************************************:
 */

#include <stdio.h>
#include <limits.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
	char *path = argv[1];
	char out[1000];
	printf("%s\n", realpath(path, out));
	return 0;
}