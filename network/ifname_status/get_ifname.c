/*
 * @FilePath: /network/ifname_status/get_ifname.c
 * @version:
 * @Author: sueRimn
 * @Date: 2020-03-29 09:46:25
 * @LastEditors: dof
 * @LastEditTime: 2022-11-11 14:15:08
 * @Descripttion: 获取所有网卡 ifname
 */

/*
 * 使用socket 获取网卡有效信息
 * ioctl https://baike.baidu.com/item/ioctl/6392403
 */

#include <arpa/inet.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>

#define MAXINTERFACES 16 /* 最大接口数 */

#if 0
int getifname(int *num, char **ifname)
{
	int fd;							 /* 套接字 */
	int if_len;						 /* 接口数量 */
	struct ifreq buf[MAXINTERFACES]; /* ifreq结构数组 */
	struct ifconf ifc;				 /* ifconf结构 */

	/* 建立IPv4的UDP套接字fd */
	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
	{
		perror("socket(AF_INET, SOCK_DGRAM, 0)");
		return -1;
	}

	/* 初始化ifconf结构 */
	ifc.ifc_len = sizeof(buf);
	ifc.ifc_buf = (caddr_t)buf;

	/* 获得接口列表  使用 SIOCGIFCONF 获取网卡信息是, 如果没有分配到ip, 就不会获取到网卡信息 */
	if (ioctl(fd, SIOCGIFCONF, (char *)&ifc) == -1)
	{
		perror("SIOCGIFCONF ioctl");
		return -1;
	}

	if_len = ifc.ifc_len / sizeof(struct ifreq); /* 接口数量 */
	printf("接口数量:%2d\n\n", if_len);
	*num = if_len;

	/* 遍历每个接口 */
	while (if_len-- != 0)
	{
		printf("%4d接口：%s\n", if_len, buf[if_len].ifr_name); /* 接口名称 */
		ifname[if_len] = strdup(buf[if_len].ifr_name);
	} // –while end

	// 关闭socket
	close(fd);
	return 0;
}

#else
int getifname(int *num, char **ifname)
{
	FILE *fd;
	char tmp[128] = {0};
	int cnt = 0;
	unsigned int pid = 0;
	char cmd[256] = {0};

	snprintf(cmd, sizeof(cmd), "ifconfig | grep mtu | awk '{print \$1}'");
	fd = popen(cmd, "r");
	if (!fd)
	{
		pclose(fd);
		return 0;
	}

	while (cnt = fgets(tmp, sizeof(tmp), fd))
	{
		printf("tmp = %s\n ", tmp);
		if (!cnt)
		{
			pclose(fd);
			return 0;
		}
	}
	pclose(fd);
	
	return 0;
}
#endif


int main(int argc, char const *argv[])

{
	char *ifname[16] = {{0}};
	int num = 0;

	char cmd[128] = "ifconfig | grep mtu | awk \'{print \$1}\'";
	printf("cmd = %s\n", cmd);

	getifname(&num, ifname);
	for (int i = 0; i < num; i++)
	{
		printf("ifname = %s\n", ifname[i]);
		if (ifname[i])
			free(ifname[i]);
	}

	return 0;
}