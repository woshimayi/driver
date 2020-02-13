#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/select.h>
#include <time.h>

#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <net/if.h>
#include <netinet/in.h>
#include <ctype.h>


static volatile int stop = 0;

void handler(int sig __attribute__((unused)))
{
    stop = 1;
}

int www_ParseUrl(const char *url, char *proto, char *host, int *port, char *uri)
{
    int n;
    char *p;

    *port = 0;
    if (NULL == url)
    {
        return 0;
    }

    if (NULL != uri)
    {
        strncpy(uri, "", 8);
    }

    /* proto */
    p = (char *) url;
    if ((p = strchr(url, ':')) == NULL)
    {
        return -1;
    }
    n = p - url;
    if (NULL != proto)
    {
        strncpy(proto, url, n);
        proto[n] = '\0';
    }

    /* skip "://" */
    if (*p++ != ':')
        return -1;
    if (*p++ != '/')
        return -1;
    if (*p++ != '/')
        return -1;

    /* host */
    {
        char *hp = host;

        while (*p && *p != ':' && *p != '/')
        {
            *hp++ = *p++;
        }
        *hp = '\0';
    }
    if (strlen(host) == 0)
        return -1;

    /* end */
    if (*p == '\0')
    {
        *port = 0;
        if (NULL != uri)
        {
            strncpy(uri, "", 8);
        }
        return 0;
    }

    /* port */
    if (*p == ':')
    {
        char buf[10];
        char *pp = buf;

        p++;
        while (isdigit(*p))
        {
            *pp++ = *p++;
        }
        *pp = '\0';
        if (strlen(buf) == 0)
            return -1;
        *port  = atoi(buf);
    }

    /* uri */
    if (*p == '/')
    {
        if (NULL != uri)
        {
            char *up = uri;
            while ((*up++ = *p++));
        }
    }
    return 0;
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

int connect_to(struct addrinfo *addr, const char *ifname)
{
    int fd;
    struct timeval start;
    int connect_result;
    const int on = 1;
    int flags;
    struct ifreq ifr;

    /* try to connect for each of the entries: */
    while (addr != NULL)
    {
        /* create socket */
        if ((fd = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol)) == -1)
            goto next_addr0;

        strncpy(ifr.ifr_name, ifname, strlen(ifname)+1);
        if (setsockopt(fd, SOL_SOCKET, SO_BINDTODEVICE, &ifr, sizeof(ifr))  < 0)
        {
            perror("SO_BINDTODEVICE failed");
            goto next_addr1;
        }
#if 1
        flags = fcntl(fd, F_GETFL, 0);
            // goto next_addr1;
        fcntl(fd, F_SETFL, flags | O_NONBLOCK);
            // goto next_addr1;
#endif

#if 0
        unsigned long ul = 1;
        ioctl(fd, FIONBIO, &ul);
#endif

        /* connect to peer */
        if ((connect_result = connect(fd, addr->ai_addr, addr->ai_addrlen)) == 0)
        {
            close(fd);
            return 0;
        }
        else 
        {
            if(errno != EINPROGRESS)
                printf("connect error :%s\n",strerror(errno));
            else
            {
                struct timeval tm   ;
                tm.tv_sec = 2;
                fd_set wset,rset;
                FD_ZERO(&wset);
                FD_ZERO(&rset);
                FD_SET(fd,&wset);
                FD_SET(fd,&rset);
                int res = select(fd+1,&rset,&wset,NULL,&tm);
                if(res < 0)
                {
                    printf("network error in connect\n");
                    goto next_addr0;
                }
                else if(res == 0)
                {
                    printf("connect time out\n");
                    close(fd);
                    return -1;
                }
                else if (1 == res)
                {
                    if(FD_ISSET(fd,&wset))
                    {
                        printf("connect succeed.\n");
                        fcntl(fd,F_SETFL,fcntl(fd,F_GETFL,0) & ~O_NONBLOCK);
                        close(fd);
                        return 0;
                    }
                    else
                    {
                        printf("other error when select:%s\n",strerror(errno));
                    }
                }
            }
        }

next_addr1:
        close(fd);
next_addr0:
        addr = addr->ai_next;
    }

    printf("errno =  %d %s\n", errno, strerror(errno));

    return -errno;
}

int tr69_ifname_test(char *hostname, char *portnr, char * ifname)
{
    int c, count = -1, curncount = 0, wait = 1, ok = 0, err = 0, errcode, ret = -1;
    struct addrinfo *resolved;

    signal(SIGINT, handler);
    signal(SIGTERM, handler);

    if ((errcode = lookup(hostname, portnr, &resolved)) != 0)
    {
        fprintf(stderr, "zx %s\n", gai_strerror(errcode));
        return 2;
    }

    printf("zx %s\n", errcode);
    if ((errcode = connect_to(resolved, ifname)) != 0)
    {
        if (errcode != -EADDRNOTAVAIL)
        {
            printf("inval address\n");
        }
    }

    if (errcode != 0)
    {
        printf("--- %s:%s unreachable ---\n", hostname, portnr);
        ret = -1;
    }
    else
    {
        printf("--- %s:%s ping statistics ---\n", hostname, portnr);
        ret = 0;
    }

    freeaddrinfo(resolved);
    return ret;
}


int main(int argc, char * argv[])
{
    int ret = -1;
    
    if (argc  < 2)
    {
        perror("param < 2");
        return 0;
    }
    char *url = "http://80.80.80.40:9090/ACS-server/ACS";
    char host[128] = {0};
    int port = 0;
    char buf[32] = {0};

    www_ParseUrl(url, NULL, host, &port, NULL);

    sprintf(buf, "%d", port);
    printf("host= %s\nport=%s\nifname=%s\n", host, buf, argv[1]);

    ret = tr69_ifname_test(host, buf, argv[1]);
    printf("ret = %d\n", ret);
    return 0;
}