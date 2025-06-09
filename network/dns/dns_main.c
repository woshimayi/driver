

// #include <stdio.h>
// #include <string.h>
// #include <stdio.h>
// #include "parse_dns.h"

// int main(int argc, char const *argv[])
// {
//     char buf[128] = {0};
//     ParaseIpv4Domain("eth0", "www.jd.com", "223.5.5.5", buf);
//     printf("buf = %s\n", buf);
//     return 0;
// }

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>

int main()
{
    struct addrinfo hints, *res, *p;
    int status;
    char ipstr[INET6_ADDRSTRLEN];

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;     // IPv4或IPv6
    hints.ai_socktype = SOCK_STREAM; // TCP套接字

    if ((status = getaddrinfo("www.baidu.com", "80", &hints, &res)) != 0)
    {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
        return 1;
    }

    printf("IP addresses for www.example.com:\n\n");

    for (p = res; p != NULL; p = p->ai_next)
    {
        void *addr;
        char *ipver;

        // 获取指向地址本身的指针
        if (p->ai_family == AF_INET)
        { // IPv4
            struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
            addr = &(ipv4->sin_addr);
            ipver = "IPv4";
        }
        else
        { // IPv6
            struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;
            addr = &(ipv6->sin6_addr);
            ipver = "IPv6";
        }

        // 将IP转换为字符串并打印
        inet_ntop(p->ai_family, addr, ipstr, sizeof(ipstr));
        printf("  %s: %s\n", ipver, ipstr);
    }

    freeaddrinfo(res); // 释放链表

    return 0;
}
