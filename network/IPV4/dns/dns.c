/*
 * @FilePath: \network\IPV4\dns\dns.c
 * @version:
 * @Author: sueRimn
 * @Date: 2020-10-10 13:53:43
 * @LastEditors: dof
 * @LastEditTime: 2024-04-12 14:09:16
 * @Descripttion: dns 解析
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

int lookup(char *host, int *ipvaddr, char *ipv6addr, uint32_t len)
{
    struct addrinfo hints;
    struct addrinfo *res, *cur;
    int ret;
    // struct sockaddr_in *addr;
    char ipbuf[128];

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC; /* Allow IPv4 */
    hints.ai_flags = AI_ALL;     /* For wildcard IP address */
    hints.ai_protocol = 0;       /* Any protocol */
    hints.ai_socktype = SOCK_STREAM;

    ret = getaddrinfo(host, NULL, &hints, &res);

    if (ret == -1)
    {
        perror("getaddrinfo");
        exit(1);
    }

    for (cur = res; cur != NULL; cur = cur->ai_next)
    {
        if (2 == cur->ai_family)
        {
            struct sockaddr_in *addr = (struct sockaddr_in *)cur->ai_addr;
            printf("[%s:%d] %s ret = %d\n", __FUNCTION__, __LINE__, inet_ntop(AF_INET, &addr->sin_addr, ipbuf, 16), inet_pton(AF_INET, ipbuf, &addr->sin_addr));
        }
        else if (10 == cur->ai_family)
        {
            struct sockaddr_in6 *addr = (struct sockaddr_in6 *)cur->ai_addr;
            printf("addr 1 = %s\n", inet_ntop(AF_INET6, &addr->sin6_addr, ipbuf, sizeof(ipbuf)));
            printf("addr 2 = %s \vret = %d\n", ipbuf, inet_pton(AF_INET6, ipbuf, &addr->sin6_addr));
        }
    }
    freeaddrinfo(res);

    *ipvaddr = inet_addr(ipbuf);
    strncpy(ipv6addr, ipbuf, len);

    return 0;
}

int main(int argc, char **argv)
{
    char domain[128] = {0};
    if (argc != 2)
    {
        printf("Usag...\n");
        strncpy(domain, "www.baidu.com", sizeof(domain));
        // exit(1);
    }
    else
    {
        strncpy(domain, argv[1], sizeof(domain));
    }

    int d = 0;
    char ipv6[128] = {0};
    printf("dns = %s\n", domain);
    system("date");
    lookup(domain, &d, ipv6, sizeof(ipv6));
    system("date");
    printf("zzzz %d\n", d);
}
