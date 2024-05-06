/*
 * @*************************************:
 * @FilePath: /user/C/string/strtok_r.c
 * @version:
 * @Author: dof
 * @Date: 2021-10-20 19:33:39
 * @LastEditors: dof
 * @LastEditTime: 2023-08-11 16:49:19
 * @Descripttion:  字符串 多 字符切割
 * @**************************************:
 */

#include <stdio.h>
#include <string.h>

int igdCmWanResolveIPv6DnsServerName(char *pDnsServsrs)
{
    char *seps = ",", *token = NULL;
    char DnsServers[256] = {0};
    int i = 0, lRet = 0;

    if (pDnsServsrs == NULL) 
	{
        return -1;
    }

    strncpy(DnsServers, pDnsServsrs, sizeof(DnsServers));
    token = strtok(DnsServers, seps);
    while (token) {
        if (i == 3) 
		{
            break;
        }

        token = strtok(NULL, seps);
        i += 1;
    }
	printf("i = %d\n", i);
	return i;
}


int main(void)
{
#if 0
	int j, in = 0;
	char buffer[100] = "Fred male 25,John male 62,Anna female 16";
	char *p[20];
	char *buf = buffer;
	char *outer_ptr = NULL;
	char *inner_ptr = NULL;
	while ((p[in] = strtok_r(buf, ",", &outer_ptr)) != NULL)
	{
		buf = p[in];
		while ((p[in] = strtok_r(buf, " ", &inner_ptr)) != NULL)
		{
			in++;
			buf = NULL;
		}
		buf = NULL;
	}
	printf("Here we have %d strings\n", in);
	for (j = 0; j < in; j++)
	{
		printf(">%s<\n", p[j]);
	}
#endif
	//int i = igdCmWanResolveIPv6DnsServerName("3000::2,3000::5,");
	//printf("i = %d\n", i);
    char *str = "/RMS-server/RMS?sn=SCTY18321B39";
	printf("i = %s\n", str);
    char *p ;
    p = strtok(str, "?");
	printf("i = %s\n", p);
	return 0;
}

