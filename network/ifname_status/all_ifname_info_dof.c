/*
 * @FilePath: /network/ifname_status/all_ifname_info.c
 * @version:
 * @Author: sueRimn
 * @Date: 2020-03-29 09:46:25
 * @LastEditors: dof
 * @LastEditTime: 2023-09-04 11:22:46
 * @Descripttion:
 */

/*
 * 使用socket 获取网卡有效信息
 * ioctl https://baike.baidu.com/item/ioctl/6392403
 *
 * 网络接口信息, 如下所示：
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
#include <linux/sockios.h>
#include <linux/ethtool.h>

#define MAXINTERFACES 16 /* 最大接口数 */

int get_max_link_speed(const char *interface_name)
{
    int fd;
    struct ifreq ifr;
    struct ethtool_cmd edata;

    // 打开套接字
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0)
    {
        perror("socket");
        return -1;
    }

    // 获取接口信息
    strncpy(ifr.ifr_name, interface_name, IFNAMSIZ - 1);
    ifr.ifr_data = (char *)&edata;
    edata.cmd = ETHTOOL_GSET;

    if (ioctl(fd, SIOCETHTOOL, &ifr) == -1)
    {
        // perror("ioctl");
        close(fd);
        return -1;
    }

    // 提取最大速率
    int speed = ethtool_cmd_speed(&edata);

    // 关闭套接字
    close(fd);

    return speed;
}

int main(int argc, char *argv[])
{
    int fd;                          /* 套接字 */
    int if_len;                      /* 接口数量 */
    struct ifreq buf[MAXINTERFACES]; /* ifreq结构数组 */
    struct ifconf ifc;               /* ifconf结构 */

    /* 建立IPv4的UDP套接字fd */
    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    {
        perror("socket(AF_INET, SOCK_DGRAM, 0)");
        return -1;
    }

    /* 初始化ifconf结构 */
    ifc.ifc_len = sizeof(buf);
    ifc.ifc_buf = (caddr_t)buf;

    /* 获得接口列表  使用 SIOCGIFCONF 获取网卡信息是, 如果没有分配到ip, 就不会获取到网卡信息 */
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
        int speed = get_max_link_speed(buf[if_len].ifr_name);
        if (0 < speed)
        {
            printf("速度：%dMbps\n", speed); /* 接口名称 */
        }

        /* 获得接口标志 */
        if (!(ioctl(fd, SIOCGIFFLAGS, (char *)&buf[if_len])))
        {
            /* 接口状态 */
            if (buf[if_len].ifr_flags & IFF_UP)
            {
                printf("接口状态: UP\n");
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
    } // –while end

    // 关闭socket
    close(fd);
    return 0;
}
