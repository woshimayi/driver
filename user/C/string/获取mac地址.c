/*
 * @*************************************:
 * @FilePath: /user/C/string/获取mac地址.c
 * @version:
 * @Author: dof
 * @Date: 2021-10-20 19:33:39
 * @LastEditors: dof
 * @LastEditTime: 2021-12-07 14:05:01
 * @Descripttion:根据 ifconfig 输出 获取mac 地址
 * @**************************************:
 */
#include<stdio.h>
#include<stdlib.h>
#include<string.h>

int main()
{
	char mac[64] = {0};
	char *p = NULL;
	FILE *fd = NULL;
	char line[128] = {0};
	fd = fopen("./ifconfig.log", "r");
	if (NULL == fd)
	{
		printf("fail ifocnig ifname\n");
	}
	else
	{
		while (fgets(line, sizeof(line), fd) != NULL)
		{
			if ((p = strstr(line, "pon")))
			{
				if (p = strstr(line, "HWaddr"))
				{
					printf("p = %s\n", p);
				}
				sscanf(p, "%*s%s", mac);
			}
		}
		printf("mac = %s\n", mac);
	}
	fclose(fd);
	return 0;
}