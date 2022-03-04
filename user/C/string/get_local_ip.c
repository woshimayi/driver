/*
 * @*************************************:
 * @FilePath: /user/C/string/get_local_ip.c
 * @version:
 * @Author: dof
 * @Date: 2022-03-04 13:13:24
 * @LastEditors: dof
 * @LastEditTime: 2022-03-04 13:13:27
 * @Descripttion: 获取本地IP地址
 * @**************************************:
 */

#include <stdio.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>

int get_local_ip(char *ps8IpList)
{
	struct ifaddrs *ifAddrStruct;
	char l_s8IpAddr[INET_ADDRSTRLEN];
	void *tmpAddrPtr;
	int l_s32IPCount = 0;

	getifaddrs(&ifAddrStruct);
	while (ifAddrStruct != NULL)
	{
		if (ifAddrStruct->ifa_addr->sa_family == AF_INET)
		{
			tmpAddrPtr = &((struct sockaddr_in *)ifAddrStruct->ifa_addr)->sin_addr;
			inet_ntop(AF_INET, tmpAddrPtr, l_s8IpAddr, INET_ADDRSTRLEN);
			if (strcmp(l_s8IpAddr, "127.0.0.1") != 0)
			{
				if (l_s32IPCount == 0)
				{
					memcpy(ps8IpList, l_s8IpAddr, INET_ADDRSTRLEN);
				}
				else
				{
					memcpy(ps8IpList + INET_ADDRSTRLEN, l_s8IpAddr, INET_ADDRSTRLEN);
				}
				l_s32IPCount++;
			}
		}
		ifAddrStruct = ifAddrStruct->ifa_next;
	}

	freeifaddrs(ifAddrStruct);
	return l_s32IPCount;
}

int main()
{
	char l_arrs8IpAddrList[3][INET_ADDRSTRLEN];
	int l_s32AddrCount;

	memset(l_arrs8IpAddrList, 0, sizeof(l_arrs8IpAddrList));

	l_s32AddrCount = get_local_ip(*l_arrs8IpAddrList);

	for (l_s32AddrCount; l_s32AddrCount > 0; l_s32AddrCount--)
	{
		printf("Server Local IP%d: %s\n", l_s32AddrCount, l_arrs8IpAddrList[l_s32AddrCount - 1]);
	}

	return 0;
}