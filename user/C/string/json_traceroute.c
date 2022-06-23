/*
 * @*************************************:
 * @FilePath: /user/C/string/json_traceroute.c
 * @version:
 * @Author: dof
 * @Date: 2022-06-23 13:17:02
 * @LastEditors: dof
 * @LastEditTime: 2022-06-23 14:50:35
 * @Descripttion:
 * @**************************************:
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define TRACEROUTE_FILE "./bu_traceroute"

void bucpe_get_traceroute_result()
{
	FILE *fs;
	char ip[16];
	char resp_ms[3][32];
	char line[256];
	char json_buf[4096];
	char *p;
#define GET_MS(x) x[0] == '*' ? "NULL" : x
#define GET_IP(y) (0 == strlen(y))?"*":y

	fs = fopen("./bu_traceroute", "r");
	if (NULL == fs)
	{
		printf("can't open %s\n", TRACEROUTE_FILE);
		return 0;
	}

	p = json_buf;
	p[0] = '\0';
	p += sprintf(p, "[");
	while (fgets(line, sizeof(line), fs))
	{
		printf("zzzzz line = %s", line);
		ip[0] = '\0';
		memset(resp_ms, 0, sizeof(resp_ms));
		sscanf(line, "%s %s %s %s", ip, resp_ms[0], resp_ms[1], resp_ms[2]);
		printf("\t\t\t\t%d ,%s-%s-%s-%s\n", strlen(ip), ip, resp_ms[0], resp_ms[1], resp_ms[2]);
		
		if ('|' == ip[0])
		{
			printf("---- 0 %c", ip[0]);
			strcpy(ip, "*");
		}
		else
		{
			int i = 0;
			while (ip[i])
			{
				// printf(" %c", ip[i]);
				if ('|' == ip[i])
				{
					printf("---- %c", ip[i]);
					ip[i] = '\0';
				}
				i++;
			}
			printf("ip =  %s\n", ip);
		}

		p += sprintf(p, "[\"%s\", %s, \"%s\", %s, \"%s\", %s],", ip, resp_ms[0], ip, resp_ms[1], ip, resp_ms[2]);
	}

	p--;
	if (p[0] == ',')
	{
		p[0] = '\0'; /*remove the last ','*/
	}
	p += sprintf(p, "]");
	printf("zzzzz p = %s\n", p);
	fclose(fs);
	//	unlink(TRACEROUTE_FILE);

	printf("buf = %s\n", json_buf);
}

int main(int argc, char const *argv[])
{
	/* code */
	bucpe_get_traceroute_result();
	return 0;
}
