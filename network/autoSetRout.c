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

#ifdef linux
	#define NONE          "\033[m"
	#define RED           "\033[0;32;31m"
	#define LIGHT_RED     "\033[1;31m"
	#define GREEN         "\033[0;32;32m"
	#define LIGHT_GREEN   "\033[1;32m"
	#define BLUE          "\033[0;32;34m"
	#define LIGHT_BLUE    "\033[1;34m"
	#define DARY_GRAY     "\033[1;30m"
	#define CYAN          "\033[0;36m"
	#define LIGHT_CYAN    "\033[1;36m"
	#define PURPLE        "\033[0;35m"
	#define LIGHT_PURPLE "\033[1;35m"
	#define BROWN         "\033[0;33m"
	#define YELLOW        "\033[1;33m"
	#define LIGHT_GRAY    "\033[0;37m"
	#define WHITE         "\033[1;37m"
#endif // __LINUX__


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

    /* try to connect for each of the entries: */
    while (addr != NULL)
    {
        /* create socket */
        if ((fd = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol)) == -1)
            goto next_addr0;

        strncpy(ifr.ifr_name, ifname, strlen(ifname) + 1);
        if (setsockopt(fd, SOL_SOCKET, SO_BINDTODEVICE, &ifr, sizeof(ifr)) < 0) // setsockopt:用来设置fd 的socket状态， SOL_SOCKET:套接字级别， SO_BINDTODEVICE:将套接字绑定到一个特定的设备上
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
                // printf("res = %d\n", res);
                if (res < 0)
                {
                    perror("connect_to");
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

    printf("hostname_acs=%s, port_acs = %s\n", hostname_acs, port_acs);
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
        printf("--- %s:%s unreachable ---\n", hostname_acs, port_acs);
        ret = -1;
    }
    else
    {
        printf("--- %s:%s ping statistics ---\n", hostname_acs, port_acs);
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

        /* IP地址 */
        if (!(ioctl(fd, SIOCGIFADDR, (char *)&buf[if_len])))
        {
            printf("IP地址:%s\n",
                   (char *)inet_ntoa(((struct sockaddr_in *)(&buf[if_len].ifr_addr))->sin_addr));
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
            printf("子网掩码:%s\n",
                   (char *)inet_ntoa(((struct sockaddr_in *)(&buf[if_len].ifr_addr))->sin_addr));
            snprintf(ifnameinfo->submask, sizeof(ifnameinfo->submask), "%s", (char *)inet_ntoa(((struct sockaddr_in *)(&buf[if_len].ifr_addr))->sin_addr));
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
            printf("ifr_hwaddr = %x\n", buf[if_len].ifr_hwaddr);
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
    } // –while end

    // 关闭socket
    close(fd);
    return 0;
}

int main(int argc, char const *argv[])
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
        printf("ifname = %s, ifnameIp = %s, ifnameMac = %s, submask = %s", ifnameinfo.ifname, ifnameinfo.ifnameIp, ifnameinfo.ifnameMac, ifnameinfo.submask);
    }

    // 1：添加指定地址的路由
    // {
    //     UTIL_DO_SYSTEM_ACTION("mkdir -p /var/iproute2");
    //     UTIL_DO_SYSTEM_ACTION("echo 100 tr69 >> /var/iproute2/rt_tables");
    //     UTIL_DO_SYSTEM_ACTION("ip ru del table %s", ACS_TR69C);
    //     UTIL_DO_SYSTEM_ACTION("ip ru add to %s table %s", hostname_acs, ACS_TR69C);
    //     UTIL_DO_SYSTEM_ACTION("ip ro replace %s via %s dev %s table %s", hostname_acs, ipAddress, obj->X_BROADCOM_COM_BoundIfName, ACS_TR69C);
    // }
    // 2：添加默认路由
    // {
    //     system("ip ro del default");
    //     system("ip ro add default via gatway ifname");
    // }

    return 0;
}
