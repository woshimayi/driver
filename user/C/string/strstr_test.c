/*
 * @*************************************:
 * @FilePath: /user/C/string/strstr_test.c
 * @version:
 * @Author: dof
 * @Date: 2023-07-10 09:42:08
 * @LastEditors: dof
 * @LastEditTime: 2024-03-20 14:03:33
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

#define PP(fmt,args...) printf("[mdm :%s(%d)] " fmt "\r\n", __func__, __LINE__, ##args )

#ifndef PP(fmt,args...)
#if 0
#define PP(fmt,args...) printf("[mdm :%s(%d)] " fmt "\r\n", __func__, __LINE__, ##args )
#else
#define PP(fmt,args...)
#endif
#endif



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

	#define isspace0(c)	((c) == ' ')
	char *separator = NULL;
	char *str = "InternetGatewayDevice.WANDevice.1.WANConnectionDevice.1.WANPPPConnection.4.Username";
	char str1[256] = {0};
	char dnsSecondary[64] = {0};
	// sscanf("2010::ca6c:20ff:fec4:ee9d/64", "%s/%*d", str);

	// printf("1 str = %s--\n", strstr(str, "Username"));
	// printf("2 str = %s--\n", strtok(str, "."));
	char *ret = strrchr(str, '.');
	snprintf(str1, ret - str + 2, "%s", str);
	PP("%p", str);
	PP("%p", ret);
	printf("3 str = %s-- \v%s\n", ret, str1);
	PP("str1 = %s", str1);

	// separator = strtok(str, ",");

	// if (separator != NULL)
    // {
	// 	/* break the string into two strings */
	// 	// *separator = 0;
	// 	 while ((isspace0(*separator)) && (*separator != 0))
	// 	 {
	// 		/* skip white space after comma */
	// 		separator++;
	// 	 }
	// 	// separator++;

	// 	strcpy(dnsSecondary, separator);
	// 	printf("dnsSecondary=%s--\n", dnsSecondary);
    // }

	printf("0x%x\n",  0x11111111 & ~0x10000000);

	char *s = NULL;
	if (s && 0 == strcasecmp(s, "FALse"))
	{
		char  t_v[8] = {0};
		void **v = &t_v[0];
		*v = (void *)1;
		printf("sss v = %d\n", (int *)(*v));
	}
	// printf("%s\n", strrchr("InternetGatewayDevice.LANDevice.1.Hosts.Host.1.", '.'));

	return 0;
}
