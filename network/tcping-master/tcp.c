#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>

#include "tcp.h"

int lookup_6(char *host, char *portnr, struct addrinfo *res)
{
    struct addrinfo hints;
    // struct addrinfo *cur;
    int ret;
    
    // struct sockaddr_in6 *addr;
    
    // char ipbuf[128];

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET6; /* Allow IPv4 */
    hints.ai_flags = AI_ALL; /* For wildcard IP address */
    hints.ai_protocol = 0; /* Any protocol */
    hints.ai_socktype = SOCK_DGRAM;

    ret = getaddrinfo(host, portnr, &hints, &res);

    if (ret == -1)
    {
        perror("getaddrinfo");
    }

    // for (cur = res; cur != NULL; cur = cur->ai_next)
    // {
        // addr = (struct sockaddr_in *)cur->ai_addr;
        // printf("addr 1 = %s\n", inet_ntop(AF_INET6, &addr->sin6_addr, ipbuf, sizeof(ipbuf)));
        // printf("addr 2 = %s ret = %d\n", ipbuf, inet_pton(AF_INET6, ipbuf, &addr->sin6_addr));
    // }
    // freeaddrinfo(res);

    return ret;
}

int lookup(char *host, char *portnr, struct addrinfo **res)
{
    struct addrinfo hints;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_NUMERICSERV;
    hints.ai_protocol = 0;

    return getaddrinfo(host, portnr, &hints, res);
}

int connect_to(struct addrinfo *addr, struct timeval *rtt)
{
    int fd;
    struct timeval start;
    int connect_result;
    const int on = 1;
    /* int flags; */
    int rv = 0;

    /* try to connect for each of the entries: */
    while (addr != NULL)
    {
        /* create socket */
        if ((fd = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol)) == -1)
            goto next_addr0;
        if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0)   // SO_REUSEADDR: 是让端口释放后立即就可以被再次使用
            goto next_addr1;
#if 0
        if ((flags = fcntl(fd, F_GETFL, 0)) == -1)
            goto next_addr1;
        if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
            goto next_addr1;
#endif
        if (gettimeofday(&start, NULL) == -1)
            goto next_addr1;

        /* connect to peer */
        if ((connect_result = connect(fd, addr->ai_addr, addr->ai_addrlen)) == 0)
        {
            if (gettimeofday(rtt, NULL) == -1)
                goto next_addr1;
            rtt->tv_sec = rtt->tv_sec - start.tv_sec;
            rtt->tv_usec = rtt->tv_usec - start.tv_usec;
            close(fd);
            return 0;
        }
        else
        {
            perror("connect fail");
        }

next_addr1:
    perror("setsocketopt fail");
    close(fd);
next_addr0:
    perror("socket fail");
    addr = addr->ai_next;
    }

    rv = rv ? rv : -errno;
    return rv;
}
