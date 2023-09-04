/*
 * @*************************************:
 * @FilePath: /network/get_ifname_speed.c
 * @version:
 * @Author: dof
 * @Date: 2023-08-31 10:33:44
 * @LastEditors: dof
 * @LastEditTime: 2023-09-04 11:06:45
 * @Descripttion:  获取接口协商最大速率
 * 					接口 状态
 * 					地址
 * 					mac
 * 
 * @**************************************:
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <linux/sockios.h>
#include <linux/ethtool.h>

int get_max_link_speed(const char *interface_name)
{
	int fd;
	struct ifreq ifr;
	struct ethtool_cmd edata;

	// 打开套接字
	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd < 0)
	{
		perror("socket");
		return -1;
	}

	// 获取接口信息
	strncpy(ifr.ifr_name, interface_name, IFNAMSIZ - 1);
	ifr.ifr_data = (char *)&edata;
	edata.cmd = ETHTOOL_GSET;

	if (ioctl(fd, SIOCETHTOOL, &ifr) == -1)
	{
		perror("ioctl");
		close(fd);
		return -1;
	}

	// 提取最大速率
	int speed = ethtool_cmd_speed(&edata);

	// 关闭套接字
	close(fd);

	return speed;
}

// #define LAN(i) "LAN"##i

int main()
{
	const char *interface_name = "eth0";
	int max_speed = get_max_link_speed(interface_name);
	if (max_speed >= 0)
	{
		printf("网卡 %s 的最大速率为 %d Mbps\n", interface_name, max_speed);
	}
	else
	{
		printf("无法获取网卡 %s 的最大速率\n", interface_name);
	}

	return 0;
}
