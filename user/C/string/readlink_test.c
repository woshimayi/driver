/*
 * @*************************************: 
 * @FilePath: /user/C/string/readlink_test.c
 * @version: 
 * @Author: dof
 * @Date: 2023-03-15 13:17:32
 * @LastEditors: dof
 * @LastEditTime: 2023-03-17 09:50:54
 * @Descripttion: 获取自身全路径
 * @**************************************: 
 */

#include <unistd.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char const *argv[])
{
	char buf[256] = {0};
	int size = 0;
	size = readlink("/proc/self/exe", buf, sizeof(buf));
	printf("buf = %s size = %d\n", buf, size);
	return 0;
}
