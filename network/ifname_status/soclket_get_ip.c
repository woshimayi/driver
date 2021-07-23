/*
 * @FilePath: /network/soclket_get_ip.c
 * @version: 
 * @Author: sueRimn
 * @Date: 2020-03-29 09:46:25
 * @LastEditors: sueRimn
 * @LastEditTime: 2021-02-25 17:02:26
 * @Descripttion: 
 */

/*
* 使用socket 获取网络信息
* ioctl https://baike.baidu.com/item/ioctl/6392403
*
* 网络接口信息
*
* 接口数量:5
* 接口：docker0
* 接口状态: UP
* IP地址:172.17.0.1
* 子网掩码:255.255.0.0
* 广播地址:0.0.0.0
* MAC地址:02:42:3d:e3:4a:04
*
*/

#include <arpa/inet.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <netdb.h>
#include <fcntl.h>

#define MAXINTERFACES 16 /* 最大接口数 */

int fd;                          /* 套接字 */
int if_len;                      /* 接口数量 */
struct ifreq buf[MAXINTERFACES]; /* ifreq结构数组 */
struct ifconf ifc;               /* ifconf结构 */
static volatile int stop = 0;
static char hostname_acs[64] = "www.baidu.com";
static char port_acs[32] = "80";

void handler(int sig __attribute__((unused)))
{
    stop = 1;
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
    int connect_result;
    int flags;
    struct ifreq ifr;

    /* try to connect for each of the entries: */
    while (addr != NULL)
    {
        /* create socket */
        if ((fd = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol)) == -1)
            goto next_addr0;

        strncpy(ifr.ifr_name, ifname, strlen(ifname) + 1);
        if (setsockopt(fd, SOL_SOCKET, SO_BINDTODEVICE, &ifr, sizeof(ifr)) < 0) //setsockopt:用来设置fd 的socket状态， SOL_SOCKET:套接字级别， SO_BINDTODEVICE:将套接字绑定到一个特定的设备上
        {
            perror("SO_BINDTODEVICE failed");
            goto next_addr1;
        }
#if 1
        flags = fcntl(fd, F_GETFL, 0);
        fcntl(fd, F_SETFL, flags | O_NONBLOCK);
#endif
        /* connect to peer */
        if ((connect_result = connect(fd, addr->ai_addr, addr->ai_addrlen)) == 0)
        {
            close(fd);
            return 0;
        }
        else
        {
            if (errno != EINPROGRESS)
                printf("connect error :%s", strerror(errno));
            else
            {
                struct timeval tm;
                tm.tv_sec = 2;
                fd_set wset, rset;
                FD_ZERO(&wset);
                FD_ZERO(&rset);
                FD_SET(fd, &wset);
                FD_SET(fd, &rset);
                int res = select(fd + 1, &rset, &wset, NULL, &tm);
                printf("res = %d\n", res);
                if (res < 0)
                {
                    printf("network error in connect\n");
                    goto next_addr0;
                }
                else if (0 == res)
                {
                    printf("connect time out\n");
                    close(fd);
                    return -1;
                }
                else if (1 == res)
                {
                    if (FD_ISSET(fd, &wset))
                    {
                        printf("connect succeed.\n");
                        fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) & ~O_NONBLOCK);
                        close(fd);
                        return 0;
                    }
                    else
                    {
                        printf("other error when select:%s\n", strerror(errno));
                    }
                }
            }
        }

    next_addr1:
        close(fd);
    next_addr0:
        addr = addr->ai_next;
    }

    return -1;
}

/**
 * [ifname_test 测试ifname接口ping 通状态]
 *
 * @param   char  ifname  [ifname 接口名称]
 *
 * @return  int           [return 0:success]
 */
int ifname_test(const char *ifname)
{
    int errcode, ret;
    struct addrinfo *resolved;

    signal(SIGINT, handler);
    signal(SIGTERM, handler);

    printf("hostname_acs=%s, port_acs = %s", hostname_acs, port_acs);
    if ((errcode = lookup(hostname_acs, port_acs, &resolved)) != 0)
    {
        fprintf(stderr, "%s\n", gai_strerror(errcode));
        return 2;
    }

    if ((errcode = connect_to(resolved, ifname)) != 0)
    {
        if (errcode != -EADDRNOTAVAIL)
        {
            printf("inval address\n");
        }
        printf("--- %s:%s unreachable ---", hostname_acs, port_acs);
        ret = -1;
    }
    else
    {
        printf("--- %s:%s ping statistics ---", hostname_acs, port_acs);
        ret = 0;
    }

    freeaddrinfo(resolved);
    return ret;
}

int main(int argc, char *argv[])
{
    /* 建立IPv4的UDP套接字fd */
    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    {
        perror("socket(AF_INET, SOCK_DGRAM, 0)");
        return -1;
    }

    /* 初始化ifconf结构 */
    ifc.ifc_len = sizeof(buf);
    ifc.ifc_buf = (caddr_t)buf;

    /* 获得接口列表 */
    if (ioctl(fd, SIOCGIFCONF, (char *)&ifc) == -1)
    {
        perror("SIOCGIFCONF ioctl");
        return -1;
    }

    if_len = ifc.ifc_len / sizeof(struct ifreq); /* 接口数量 */
    printf("接口数量:%d\n\n", if_len);

    /* 遍历每个接口 */
    while (if_len-- != 0)
    {
        printf("接口：%s\n", buf[if_len].ifr_name); /* 接口名称 */

        /* 获得接口标志 */
        if (!(ioctl(fd, SIOCGIFFLAGS, (char *)&buf[if_len])))
        {
            /* 接口状态 */
            if (buf[if_len].ifr_flags & IFF_UP)
            {
                printf("接口状态: UP\n");
                if (0 == strncmp(buf[if_len].ifr_name, "lo", sizeof(buf[if_len].ifr_name)))
                {
                    continue;
                }
                tr69_ifname_test(buf[if_len].ifr_name);
            }
            else
            {
                printf("接口状态: DOW\n");
            }
        }
        else
        {
            char str[256];
            sprintf(str, "SIOCGIFFLAGS ioctl %s\n", buf[if_len].ifr_name);
            perror(str);
        }

        /* IP地址 */
        if (!(ioctl(fd, SIOCGIFADDR, (char *)&buf[if_len])))
        {
            printf("IP地址:%s\n",
                   (char *)inet_ntoa(((struct sockaddr_in *)(&buf[if_len].ifr_addr))->sin_addr));
        }
        else
        {
            char str[256];
            sprintf(str, "SIOCGIFADDR ioctl %s\n", buf[if_len].ifr_name);
            perror(str);
        }

        /* 子网掩码 */
        if (!(ioctl(fd, SIOCGIFNETMASK, (char *)&buf[if_len])))
        {
            printf("子网掩码:%s\n",
                   (char *)inet_ntoa(((struct sockaddr_in *)(&buf[if_len].ifr_addr))->sin_addr));
        }
        else
        {
            char str[256];
            sprintf(str, "SIOCGIFADDR ioctl %s\n", buf[if_len].ifr_name);
            perror(str);
        }

        /* 广播地址 */
        if (!(ioctl(fd, SIOCGIFBRDADDR, (char *)&buf[if_len])))
        {
            printf("广播地址:%s\n",
                   (char *)inet_ntoa(((struct sockaddr_in *)(&buf[if_len].ifr_addr))->sin_addr));
        }
        else
        {
            char str[256];
            sprintf(str, "SIOCGIFADDR ioctl %s\n", buf[if_len].ifr_name);
            perror(str);
        }

        /*MAC地址 */
        if (!(ioctl(fd, SIOCGIFHWADDR, (char *)&buf[if_len])))
        {
            printf("MAC地址:%02x:%02x:%02x:%02x:%02x:%02x\n\n",
                   (unsigned char)buf[if_len].ifr_hwaddr.sa_data[0],
                   (unsigned char)buf[if_len].ifr_hwaddr.sa_data[1],
                   (unsigned char)buf[if_len].ifr_hwaddr.sa_data[2],
                   (unsigned char)buf[if_len].ifr_hwaddr.sa_data[3],
                   (unsigned char)buf[if_len].ifr_hwaddr.sa_data[4],
                   (unsigned char)buf[if_len].ifr_hwaddr.sa_data[5]);
        }
        else
        {
            char str[256];
            sprintf(str, "SIOCGIFHWADDR ioctl %s\n", buf[if_len].ifr_name);
            perror(str);
        }
    } //–while end

    //关闭socket
    close(fd);
    return 0;
}
