/*
 * @FilePath: /network/dns_networking.c
 * @version: 
 * @Author: sueRimn
 * @Date: 2020-03-29 09:46:25
 * @LastEditors: sueRimn
 * @LastEditTime: 2021-02-25 17:02:56
 * @Descripttion: 
 */
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

int lookup(char *host, int *d)
{
    struct addrinfo hints;
    struct addrinfo *res, *cur;
    int ret;
    struct sockaddr_in *addr;
    char ipbuf[16];

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET; /* Allow IPv4 */
    hints.ai_flags = AI_PASSIVE; /* For wildcard IP address */
    hints.ai_protocol = 0; /* Any protocol */
    hints.ai_socktype = SOCK_STREAM;

    ret = getaddrinfo(host, NULL, &hints, &res);

    if (ret == -1)
    {
        perror("getaddrinfo");
        return ret;
    }

    for (cur = res; cur != NULL; cur = cur->ai_next)
    {
        addr = (struct sockaddr_in *)cur->ai_addr;
        //printf("%s\n",
        inet_ntop(AF_INET, &addr->sin_addr, ipbuf, 16);
    }
    freeaddrinfo(res);
    *d = inet_addr(ipbuf);
    return ret;
}


int main(int argc, char **argv)
{
    if (argc != 2)
    {
        argv[1] = strdup("www.baidu.com");
    }

    int d = 0;
    if (0 != lookup(argv[1], &d))
    {
        printf("N\n");
    }
    else
    {
        printf("%d\n", d);
        printf("Y\n");
    }
}
