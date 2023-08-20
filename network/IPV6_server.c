/*
 * @*************************************: 
 * @FilePath: /network/IPV6_server.c
 * @version: 
 * @Author: dof
 * @Date: 2023-08-18 15:39:08
 * @LastEditors: dof
 * @LastEditTime: 2023-08-18 16:22:30
 * @Descripttion: 监听ipv4 ipv6 任意地址
 * @**************************************: 
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main() {
  // 创建IPv4套接字
  int ipv4_sock = socket(AF_INET, SOCK_STREAM, 0);
  if (ipv4_sock == -1) {
    perror("IPv4 socket");
    exit(1);
  }

  // 创建IPv6套接字
  int ipv6_sock = socket(AF_INET6, SOCK_STREAM, 0);
  if (ipv6_sock == -1) {
    perror("IPv6 socket");
    exit(1);
  }

  // 设置套接字选项 SO_REUSEADDR 和 SO_REUSEPORT
  int reuse = 1;
  if (setsockopt(ipv4_sock, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) == -1) {
    perror("setsockopt (IPv4)");
    exit(1);
  }

	//  绑定IPv6地址并禁用IPv4映射
	int option = 1;
	if (setsockopt(ipv6_sock, IPPROTO_IPV6, IPV6_V6ONLY, &option, sizeof(option)) == -1) {
	perror("setsockopt (IPv6 only)");
	exit(1);
	}

  if (setsockopt(ipv6_sock, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) == -1) {
    perror("setsockopt (IPv6)");
    exit(1);
  }

  // 绑定IPv4地址
  struct sockaddr_in ipv4_addr;
  memset(&ipv4_addr, 0, sizeof(ipv4_addr));
  ipv4_addr.sin_family = AF_INET;
  ipv4_addr.sin_port = htons(8080);
  ipv4_addr.sin_addr.s_addr = INADDR_ANY;

  if (bind(ipv4_sock, (struct sockaddr*)&ipv4_addr, sizeof(ipv4_addr)) == -1) {
    perror("IPv4 bind");
    exit(1);
  }

  // 绑定IPv6地址
  struct sockaddr_in6 ipv6_addr;
  memset(&ipv6_addr, 0, sizeof(ipv6_addr));
  ipv6_addr.sin6_family = AF_INET6;
  ipv6_addr.sin6_port = htons(8080);
  ipv6_addr.sin6_addr = in6addr_any;

  if (bind(ipv6_sock, (struct sockaddr*)&ipv6_addr, sizeof(ipv6_addr)) == -1) {
    perror("IPv6 bind");
    exit(1);
  }

  // 监听连接
  if (listen(ipv4_sock, 10) == -1) {
    perror("IPv4 listen");
    exit(1);
  }

  if (listen(ipv6_sock, 10) == -1) {
    perror("IPv6 listen");
    exit(1);
  }

  printf("Server started listening on port 8080\n");

  while (1) {
    // 接受客户端连接
    struct sockaddr_storage client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    int client_sock = accept(ipv4_sock, (struct sockaddr*)&client_addr, &client_addr_len);
    if (client_sock == -1) {
      perror("accept");
      continue;
    }

    // 处理客户端请求...

    // 关闭客户端套接字
    close(client_sock);
  }

  // 关闭套接字
  close(ipv4_sock);
  close(ipv6_sock);

  return 0;
}