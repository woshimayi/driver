/*
 * @FilePath: /network/IPV6/dns/dns6.c
 * @version: 
 * @Author: sueRimn
 * @Date: 2020-10-10 13:53:43
 * @LastEditors: dof
 * @LastEditTime: 2022-08-08 13:52:11
 * @Descripttion: dns6 解析
 */
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

int lookup(char *host, char * ipv6addr, uint32_t len)
{
    struct addrinfo hints;
    struct addrinfo *res, *cur;
    int ret;
    
    // struct sockaddr_in *addr;
    
    struct sockaddr_in6 *addr;
    
    char ipbuf[128];

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET6; /* Allow IPv4 */
    hints.ai_flags = AI_ALL; /* For wildcard IP address */
    hints.ai_protocol = 0; /* Any protocol */
    hints.ai_socktype = SOCK_DGRAM;

    ret = getaddrinfo(host, NULL, &hints, &res);

    if (ret == -1)
    {
        perror("getaddrinfo");
        exit(1);
    }

    for (cur = res; cur != NULL; cur = cur->ai_next)
    {
        addr = (struct sockaddr_in6 *)cur->ai_addr;
        printf("addr 1 = %s\n", inet_ntop(AF_INET6,
                                 &addr->sin6_addr, ipbuf, sizeof(ipbuf)));
        printf("addr 2 = %s ret = %d\n", ipbuf, inet_pton(AF_INET6, ipbuf, &addr->sin6_addr));
    }
    freeaddrinfo(res);
    strncpy(ipv6addr, ipbuf, len);
}


int main(int argc, char **argv)
{
    char domain[128] = {0};
    if (argc != 2)
    {
        printf("Usag... input ipv6 domain\n");
        // strncpy(domain, "www.baidu.com", sizeof(domain));
        exit(1);
    }
    else
    {
        strncpy(domain, argv[1], sizeof(domain));
    }

    char d[128] = {0};
    printf("dns = %s\n", domain);
    system("date");
    lookup(domain, d, sizeof(d));
    system("date");
    printf("addr = %s\n", d);

    return 0;
}
