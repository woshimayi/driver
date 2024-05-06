/*
 * @*************************************:
 * @FilePath: \network\IPV4\dns\dns_serveraddres.c
 * @version:
 * @Author: dof
 * @Date: 2024-04-12 13:57:00
 * @LastEditors: dof
 * @LastEditTime: 2024-04-12 14:34:54
 * @Descripttion:   domain 解析 指定dns 服务器地址
 * @**************************************:
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netdb.h>

int lookup(char *doMain, char *dnsAddress, int *hostip)
{
    if (NULL == doMain || NULL == dnsAddress)
    {
        return -1;
    }

    // 将域名转换为规范形式
    // char canonname[256];

    // if (getcanonname(argv[1], canonname, sizeof(canonname)) < 0) {
    //     perror("getcanonname");
    //     return -1;
    // }

    // 将 DNS 服务器地址转换为数字形式
    struct in_addr dns_addr;
    if (inet_aton(dnsAddress, &dns_addr) != 1)
    {
        fprintf(stderr, "Invalid DNS server address: %s\n", dnsAddress);
        return -1;
    }

    // 创建 addrinfo 结构体
    struct addrinfo hints, *res, *ai;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;

    // 设置 DNS 服务器地址
    struct sockaddr_in dns_sockaddr;
    dns_sockaddr.sin_family = AF_INET;
    dns_sockaddr.sin_addr = dns_addr;
    dns_sockaddr.sin_port = htons(53); // DNS 服务器端口号

    hints.ai_addr = (struct sockaddr *)&dns_sockaddr;
    hints.ai_addrlen = sizeof(dns_sockaddr);

    // 调用 getaddrinfo() 函数
    if (getaddrinfo(doMain, NULL, &hints, &res) != 0)
    {
        gai_strerror(EAI_AGAIN);
        return -1;
    }

    int i = 0;
    // 打印域名解析结果
    for (ai = res; ai != NULL; ai = ai->ai_next)
    {
        char addr_buf[128];
        if (getnameinfo(ai->ai_addr, ai->ai_addrlen, addr_buf, sizeof(addr_buf), NULL, NULL, NI_NUMERICHOST) == 0)
        {
            printf("IP address: %s\n", addr_buf);
            *hostip = inet_addr(addr_buf);
        }
    }

    freeaddrinfo(res);

    return 0;
}

int main(int argc, char const *argv[])
{
    /* code */
    int hostip = 0;
    lookup("www.jd.com", "23.5.5.5", &hostip);
    printf("hostip = %d\n", hostip);
    char ip_str[INET6_ADDRSTRLEN] = {0};
    if (inet_ntop(AF_INET, &hostip, ip_str, sizeof(ip_str)) == NULL)
    {
        perror("inet_ntop");
    }
    printf("ip = %s\n", ip_str);
    return 0;
}
