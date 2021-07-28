/*
 * @*************************************: 
 * @FilePath: /driver/network/ifname_status/autoSetRout_noifname.c
 * @version: 
 * @Author: dof
 * @Date: 2021-07-21 10:49:35
 * @LastEditors: dof
 * @LastEditTime: 2021-07-28 15:16:54
 * @Descripttion: 自动添加主机默认路由
 * @**************************************: 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/select.h>

#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <netinet/in.h>
#include <ctype.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <signal.h>

#define MDMVS_ANY_WAN "Any_WAN"
#define ACS_TR69C "tr69"

static volatile int stop = 0;
static char *URL = "http://www.baidu.com";
static char hostname_acs[64] = {0};
static char port_acs[32] = {0};

struct ifNameInfo
{
    char ifname[32];
    char ifnameIp[64];
    char ifnameMac[32];
    char gatwayIp[32];
    char submask[32];
    char dns[][32];
};

void handler(int sig __attribute__((unused)))
{
    stop = 1;
}

/**
 * [www_ParseUrl 解析url地址和端口号]
 *
 * @param   char  url    [url 输入url]
 * @param   char  proto  [proto description]
 * @param   char  host   [host description]
 * @param   int   port   [port description]
 * @param   char  uri    [uri description]
 *
 * @return  int          [return description]
 */
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
    p = (char *)url;
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
        *port = atoi(buf);
    }

    /* uri */
    if (*p == '/')
    {
        if (NULL != uri)
        {
            char *up = uri;
            while ((*up++ = *p++))
                ;
        }
    }
    return 0;
}

/**
 * [struct dns 地址转换]
 *
 * @param   char  host    [host description]
 * @param   char  portnr  [portnr description]
 */
int lookup(char *host, char *portnr __attribute__((unused)), struct addrinfo **res)
{
    struct addrinfo hints;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_NUMERICSERV;
    hints.ai_protocol = 0;

    return getaddrinfo(host, portnr, &hints, res);
}

/**
 * [struct 连接ping测试]
 */
int connect_to(struct addrinfo *addr, const char *ifname)
{
    int fd;
    int connect_result;
    int flags;
    struct ifreq ifr;
    const int on = 1;

    /* try to connect for each of the entries: */
    while (addr != NULL)
    {
        /* create socket */
        if ((fd = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol)) == -1)
            goto next_addr0;

        if (strlen(ifname))
        {
            strncpy(ifr.ifr_name, ifname, strlen(ifname) + 1);
            if (setsockopt(fd, SOL_SOCKET, SO_BINDTODEVICE, &ifr, sizeof(ifr)) < 0) //setsockopt:用来设置fd 的socket状态， SOL_SOCKET:套接字级别， SO_BINDTODEVICE:将套接字绑定到一个特定的设备上
            {
                perror("SO_BINDTODEVICE failed");
                goto next_addr1;
            }
        }
        else
        {
            if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0) // SO_REUSEADDR: 是让端口释放后立即就可以被再次使用
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
                tm.tv_sec = 5;
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



int tcping_connect(struct addrinfo *addr, struct timeval *rtt)
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

next_addr1:
        close(fd);
next_addr0:
        addr = addr->ai_next;
    }

    rv = rv ? rv : -errno;
    return rv;
}


/**
 * [ifname_test 接口ping测试]
 *
 * @param   char  ifname  [ifname description]
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
        return errcode;
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

int fromIfnameGetIp(struct ifNameInfo *ifnameinfo)
{
    int fd;               /* 套接字 */
    int if_len;           /* 接口数量 */
    struct ifreq buf[16]; /* ifreq结构数组 */
    struct ifconf ifc;    /* ifconf结构 */

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

        if (0 == strcmp(buf[if_len].ifr_name, "lo"))
        {
            continue;
        }

        snprintf(ifnameinfo->ifname, sizeof(ifnameinfo->ifname), "%s", buf[if_len].ifr_name);

        /* IP地址 */
        if (!(ioctl(fd, SIOCGIFADDR, (char *)&buf[if_len])))
        {
            printf("IP地址:%s %x\n",
                   (char *)inet_ntoa(((struct sockaddr_in *)(&buf[if_len].ifr_addr))->sin_addr), ((struct sockaddr_in *)(&buf[if_len].ifr_addr))->sin_addr);
            // ifnameinfo->ifnameIp = strdup((char *)inet_ntoa(((struct sockaddr_in *)(&buf[if_len].ifr_addr))->sin_addr));
            snprintf(ifnameinfo->ifnameIp, sizeof(ifnameinfo->ifnameIp), "%s", (char *)inet_ntoa(((struct sockaddr_in *)(&buf[if_len].ifr_addr))->sin_addr));
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
            printf("子网掩码:%s %x\n",
                   (char *)inet_ntoa(((struct sockaddr_in *)(&buf[if_len].ifr_addr))->sin_addr), ((struct sockaddr_in *)(&buf[if_len].ifr_addr))->sin_addr);
            snprintf(ifnameinfo->submask, sizeof(ifnameinfo->ifnameIp), "%s", (char *)inet_ntoa(((struct sockaddr_in *)(&buf[if_len].ifr_addr))->sin_addr));
        }
        else
        {
            char str[256];
            sprintf(str, "SIOCGIFADDR ioctl %s\n", buf[if_len].ifr_name);
            perror(str);
        }

        // 网关计算
        unsigned int ip = htonl(ntohl(inet_addr(ifnameinfo->ifnameIp) & inet_addr(ifnameinfo->submask)) + 1);

        struct in_addr addr;
        memcpy(&addr, &ip, 4);
        snprintf(ifnameinfo->gatwayIp, sizeof(ifnameinfo->gatwayIp), "%s", (char *)inet_ntoa(addr));

        /*MAC地址 */
        if (!(ioctl(fd, SIOCGIFHWADDR, (char *)&buf[if_len])))
        {
            printf("ifr_hwaddr = %x\n", buf[if_len].ifr_hwaddr.sa_data);
            snprintf(ifnameinfo->ifnameMac, sizeof(ifnameinfo->ifnameMac), "%02x:%02x:%02x:%02x:%02x:%02x",
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

        /* 获得接口标志 */
        if (!(ioctl(fd, SIOCGIFFLAGS, (char *)&buf[if_len])))
        {
            /* 接口状态 */
            if (buf[if_len].ifr_flags & IFF_UP)
            {
                printf("接口状态: UP\n");
                if (0 == ifname_test(buf[if_len].ifr_name))
                {
                    break;
                }
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
    } //–while end

    //关闭socket
    close(fd);
    return 0;
}

int setRoute(void)
{
    /* code */
    char url[256] = {0};
    char *ifname = NULL;
    int port = 0;
    int ret = -1;
    struct ifNameInfo ifnameinfo;

    www_ParseUrl(URL, NULL, hostname_acs, &port, NULL);
    snprintf(port_acs, sizeof(port_acs), "%d", port ? port : 80);

    // 获取系统指定网关ping测试结果
    // ret = tr69_ifname_test(ifnameinfo);

    // 获取系统指定网关ping测试接口信息
    ret = fromIfnameGetIp(&ifnameinfo);
    if (0 == ret)
    {
        printf("ifname = %s, ifnameIp = %s, ifnameMac = %s, submask = %s gatway = %s\n", ifnameinfo.ifname, ifnameinfo.ifnameIp, ifnameinfo.ifnameMac, ifnameinfo.submask, ifnameinfo.gatwayIp);
    }

    // 1：添加策略路由
    // {
    //     UTIL_DO_SYSTEM_ACTION("mkdir -p /var/iproute2");
    //     UTIL_DO_SYSTEM_ACTION("echo 100 tr69 >> /var/iproute2/rt_tables");
    //     UTIL_DO_SYSTEM_ACTION("ip ru del table %s", ACS_TR69C);
    //     UTIL_DO_SYSTEM_ACTION("ip ru add to %s table %s", hostname_acs, ACS_TR69C);
    //     UTIL_DO_SYSTEM_ACTION("ip ro replace %s via %s dev %s table %s", hostname_acs, ipAddress, obj->X_BROADCOM_COM_BoundIfName, ACS_TR69C);
    // }
    // 2：添加默认路由
    {
        char cmd[128] = {0};
        system("ip ro del default");
        snprintf(cmd, sizeof(cmd), "ip ro add default via %s dev %s", ifnameinfo.gatwayIp, ifnameinfo.ifname);
        system(cmd);
    }

    return 0;
}

int tcping_test(char *hostname)
{
    char *portnr = "80";
    int c;
    int curncount = 0;
    int wait = 1, quiet = 0;
    int ok = 0, err = 0;
    double min = 9999.0, avg = 0.0, max = 0.0;
    struct addrinfo *resolved;
    int errcode;
    int seen_addrnotavail;

    if ((errcode = lookup(hostname, portnr, &resolved)) != 0)
    {
        fprintf(stderr, "loopup err (%d): %s\n", errcode, gai_strerror(errcode));
        return errcode;
    }

    printf("PING %s:%s\n", hostname, portnr);

    while (1)
    {
        double ms;
        struct timeval rtt;

        if ((errcode = tcping_connect(resolved, &rtt)) != 0)
        {
            if (errcode != -EADDRNOTAVAIL)
            {
                printf("error connecting to host (%d): %s\n", -errcode, strerror(-errcode));
                err++;
            }
            else
            {
                if (seen_addrnotavail)
                {
                    printf(".");
                    fflush(stdout);
                }
                else
                {
                    printf("error connecting to host (%d): %s\n", -errcode, strerror(-errcode));
                }
                seen_addrnotavail = 1;
            }
        }
        else
        {
            seen_addrnotavail = 0;
            err = 0;
            ok++;

            ms = ((double)rtt.tv_sec * 1000.0) + ((double)rtt.tv_usec / 1000.0);
            avg += ms;
            min = min > ms ? ms : min;
            max = max < ms ? ms : max;

            printf("response from %s:%s, seq=%d time=%.2f ms\n", hostname, portnr, curncount, ms);
            if (ms > 300)
                break; /* Stop the test on the first long connect() */
        }

        // 超过三次重新设置route
        if (3 <= err)
        {
            setRoute();
            break;
        }
        sleep(wait);
    }

    freeaddrinfo(resolved);
}

#if 0
// 向标准错误输出信息，告诉用户时间到了
void prompt_info(int signo)
{
    printf("time is running out\n");
    setRoute();
}

// 建立信号处理机制
void *init_sigaction(void *args)
{
    struct sigaction tact;
    /*信号到了要执行的任务处理函数为prompt_info*/
    tact.sa_handler = prompt_info;
    tact.sa_flags = 0;
    /*初始化信号集*/
    sigemptyset(&tact.sa_mask);
    /*建立信号处理机制*/
    sigaction(SIGALRM, &tact, NULL);

    struct itimerval value;
    /*设定执行任务的时间间隔为2秒0微秒*/
    value.it_value.tv_sec = 5;
    value.it_value.tv_usec = 0;
    /*设定初始时间计数也为2秒0微秒*/
    value.it_interval = value.it_value;
    /*设置计时器ITIMER_REAL*/
    setitimer(ITIMER_REAL, &value, NULL);
}

int main()
{
    struct itimerval curr_value;
    int ret;
    init_sigaction(NULL);

    while (1)
    {
        ret = getitimer(ITIMER_REAL, &curr_value); // 获取当前定时器状态
        if (0 == ret)
        {
            printf("%d %d\n", curr_value.it_value.tv_sec, curr_value.it_interval);
        }
        sleep(1);
    }

    return 0;
}
#endif

int main(int argc, char *argv[])
{
    int ret = -1;
    char hostname[64] = "www.baidu.com";
    while (1)
    {
        ret = tcping_test(&hostname);
    }
    return 0;
}
