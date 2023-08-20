/*
 * @*************************************:
 * @FilePath: /user/C/string/strstr_test.c
 * @version:
 * @Author: dof
 * @Date: 2023-07-10 09:42:08
 * @LastEditors: dof
 * @LastEditTime: 2023-08-01 20:02:37
 * @Descripttion:
 * @**************************************:
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static void cgi_get_info_from_file(const char *name, char *buf, unsigned int len)
{
	FILE *fp;
	unsigned int ret;

	fp = fopen(name, "r");
	if (fp == NULL)
	{
		printf("%s not exit.\r\n", name);
		return;
	}

	// ret = fread(buf, 1, len, fp);
	// if (ret >= len || ret == 0)
	// {
	// 	printf("read %s error, ret = %d.\r\n", name, ret);
	// }
	while(fgets(buf, len, fp))
	{
		// printf("buf = %s\n", buf);
		if (strstr(buf, "release time"))
		{
			printf("buf = %s\n", buf);
			break;
		}
	}
	buf[len - 1] = 0;
	fclose(fp);
}

int main(int argc, char const *argv[])
{
	// char deviceinfo[1024] = {0};
	// char *release_time;
	// char buf[256] = {0};

	// cgi_get_info_from_file("/home/zs/Documents/driver/user/C/string/hi_version", deviceinfo, sizeof(deviceinfo));
	// printf("deviceinfo = %s\n", deviceinfo);
	// release_time = strstr(deviceinfo, "release time");

	// if (release_time == NULL || (release_time = strstr(release_time, ":")) == NULL)
	// {
	// 	deviceinfo[0] = 0;
	// 	release_time = deviceinfo;
	// }
	// else
	// {
	// 	release_time++;
	// 	if (strlen(release_time))
	// 	{
	// 		release_time[strlen(release_time) - 1] = 0;
	// 	}
	// }

	// printf("release time = %s\n", release_time);

	char str[] = "2010::ca6c:20ff:fec4:ee9d/64";
	// sscanf("2010::ca6c:20ff:fec4:ee9d/64", "%s/%*d", str);

	printf("str = %s\n", strtok(str, "/"));

	return 0;
}
