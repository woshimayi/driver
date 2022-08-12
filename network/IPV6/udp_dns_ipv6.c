/*
 * @*************************************:
 * @FilePath: /network/IPV6/udp_dns_ipv6.c
 * @version:
 * @Author: dof
 * @Date: 2022-08-08 11:05:44
 * @LastEditors: dof
 * @LastEditTime: 2022-08-08 13:24:01
 * @Descripttion:
 * @**************************************:
 */

**Description : IPv6 UDP套接字编程示例
					*Author : mason
								  *Date : 201808 *
	/
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

	/* IPv6压缩格式地址 */
	static char src[128] = "fd11::100";
static char dst[128] = "fd11::200";

void main(int argc, char *argv[])
{
	if (2 != argc)
	{
		printf("no input domain or ipv6 addr\n");
		return -1;
	}

	strncpy(dst, argv[1], sizeof(dst));

	int udp6_socket, ret, ttl, on, addr_len;
	struct sockaddr_in6 saddr;
	struct sockaddr_in6 daddr;
	char buffer[] = "Hello World6";

	/* 设置IPv6地址，这个函数支持三种IPv6地址格式，包括首选格式、压缩格式和映射格式*/
	if ((inet_pton(AF_INET6, (char *)&src[0], &saddr.sin6_addr)) != 1)
	{
		printf("invalid ipv6 addr \r\n");
		return;
	}

	if ((inet_pton(AF_INET6, (char *)&dst[0], &daddr.sin6_addr)) != 1)
	{
		printf("invalid ipv6 addr \r\n");
		return;
	}

	addr_len = sizeof(struct sockaddr_in6);
	saddr.sin6_family = AF_INET6;
	saddr.sin6_port = htons(0);

	daddr.sin6_family = AF_INET6;
	daddr.sin6_port = htons(53);
	if(inet_pton(AF_INET6, "www.tytest6.net", &serv_addr.sin6_addr)<0) 
	{ 
		cmsLog_error("dnsserver error");
		return -1;
	} 


	/* 创建IPv6套接字，IPv6使用AF_INET6 */
	udp6_socket = socket(AF_INET6, SOCK_DGRAM, 0);
	if (udp6_socket == -1)
	{
		printf("create udp6_socket fail\r\n");
		return;
	}

	int sockopt = 1;
	if (setsockopt(udp6_socket, SOL_SOCKET, SO_REUSEADDR, (void *)&sockopt,sizeof (sockopt)) == -1) 
	{
		cmsLog_error("fail to set sock opt SO_REUSEADDR");
		perror("fail SO_REUSEADDR");
	}

	/* 绑定地址 */
	if (bind(udp6_socket, (struct sockaddr *)&saddr, addr_len))
	{
		printf("udp6 bind addr fail, err : %d\r\n", errno);
		close(udp6_socket);
		return;
	}

	/* 发送 */
	ret = sendto(udp6_socket, &buffer[0], sizeof(buffer), 0, (struct sockaddr *)&daddr, addr_len);
	if (ret > 0)
	{
		printf("udp6 send %d bytes success \r\n", ret);
	}

	/* 关闭套接字 */
	close(udp6_socket);
	return;
}