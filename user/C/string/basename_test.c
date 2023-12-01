/*
 * @*************************************:
 * @FilePath: /user/C/string/basename_test.c
 * @version:
 * @Author: dof
 * @Date: 2023-11-08 09:36:29
 * @LastEditors: dof
 * @LastEditTime: 2023-11-08 09:39:53
 * @Descripttion: 获取路径最后的文件名： basename dirname
 * @**************************************:
 */

#include "stdio.h"
#include "libgen.h"
int main()
{
	char *p;
	p = basename("nihao/nihao/jhhh/999"); // 这里只要加上自己想填的路径即可
	printf("%s\n", p);

	// char *d;
	// d = dirname("nihao/nihao/jhhh/999");
	// printf("%s\n", d);

	char *dirc, *basec, *bname, *dname;
	char *path = "/etc/passwd";

	dirc = strdup(path);
	basec = strdup(path);
	dname = dirname(dirc);
	bname = basename(basec);
	printf("dirname=%s, basename=%s\n", dname, bname);

	return 0;
}
