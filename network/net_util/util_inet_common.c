/*
 * @*************************************: 
 * @FilePath: /network/util_inet_common.c
 * @version: 
 * @Author: dof
 * @Date: 2022-07-16 19:49:39
 * @LastEditors: dof
 * @LastEditTime: 2022-07-16 20:38:24
 * @Descripttion: 点分十进制 转换为 二进制 相互转换
 * @**************************************: 
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main(int argc, char const *argv[])
{
	int ip = 0;

	inet_pton(AF_INET, "172.16.28.84", &ip); //        点分十进制 转换为 二进制

	char str[INET_ADDRSTRLEN] = {0};
	char *ptr = inet_ntop(AF_INET, &ip, str, sizeof(str)); // 二进制转换为 点分十进制

	printf("ptr = %s str = %s 0x%x\n", ptr, str, ip);

	return 0;
}

