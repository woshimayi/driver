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
 *
 *  注意事项：
 *          1：SIOCGIFCONF	只返回有 IP 的接口，缓冲区可能太小	改用动态缓冲区或 getifaddrs()
 *          2：getifaddrs()	无	推荐使用，能获取所有接口      非线程安全(需加锁)
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
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <sys/socket.h>
#include <netdb.h>

#define MAXINTERFACES 16 /* 最大接口数 */

#define MAX_INTERFACE_NAME_LEN 64
#define MAX_IP_ADDRESS_LEN 16
#define MAX_MAC_ADDRESS_LEN 18
#define NI_MAXHOST 128

// 定义接口信息结构体
typedef struct
{
    char name[16]; // 接口名称（如 eth0）
    char ip[46];   // IPv4/IPv6 地址（字符串形式）
    char mac[18];  // MAC 地址（格式 "00:11:22:33:44:55"）
    bool is_up;    // 接口是否启用（UP/DOWN）
} InterfaceInfo;

typedef struct
{
    InterfaceInfo *(*get_all_ifname)(int *count);
    void (*interfaces_dump)(const InterfaceInfo *interfaces, int count);
} _ifname_T;

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

InterfaceInfo *get_all_interfaces(int *count)
{
    struct ifaddrs *ifaddr, *ifa;
    *count = 0;

    // 第一次遍历：计算接口数量
    if (getifaddrs(&ifaddr) == -1)
    {
        perror("getifaddrs");
        return NULL;
    }

    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
    {
        if (ifa->ifa_addr && (ifa->ifa_addr->sa_family == AF_INET || ifa->ifa_addr->sa_family == AF_INET6))
        {
            (*count)++;
        }
    }

    // 分配内存
    InterfaceInfo *interfaces = malloc(*count * sizeof(InterfaceInfo));
    if (!interfaces)
    {
        perror("malloc");
        freeifaddrs(ifaddr);
        return NULL;
    }

    // 第二次遍历：填充数据
    int idx = 0;
    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
    {
        if (!ifa->ifa_addr)
            continue;

        int family = ifa->ifa_addr->sa_family;
        if (family != AF_INET && family != AF_INET6)
            continue;

        // 初始化当前接口
        InterfaceInfo *info = &interfaces[idx];
        memset(info, 0, sizeof(InterfaceInfo));

        // 设置接口名称和状态
        strncpy(info->name, ifa->ifa_name, sizeof(info->name) - 1);
        info->is_up = (ifa->ifa_flags & IFF_UP) ? true : false;

        // 获取 IP 地址
        char host[NI_MAXHOST];
        int ret = getnameinfo(ifa->ifa_addr,
                              (family == AF_INET) ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6),
                              host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
        if (ret == 0)
        {
            strncpy(info->ip, host, sizeof(info->ip) - 1);
        }
        else
        {
            snprintf(info->ip, sizeof(info->ip), "N/A");
        }

        idx++;
    }

    freeifaddrs(ifaddr);
    return interfaces;
}

void interfaces_dump(const InterfaceInfo *interfaces, int count)
{
    printf("=== 网络接口信息（共 %d 个）===\n", count);
    for (int i = 0; i < count; i++)
    {
        printf("[%d] %-8s | IP: %-15s | %s\n",
               i,
               interfaces[i].name,
               interfaces[i].ip,
               interfaces[i].is_up ? "UP" : "DOWN");
    }
}

// #include "net_interfaces.h"
#include <stdio.h>

int main()
{
    int count = 0;
    // InterfaceInfo *interfaces = get_all_interfaces(&count);

    // if (interfaces)
    // {
    //     interfaces_dump(interfaces, count);
    //     free(interfaces); // 必须释放内存！
    // }
    // else
    // {
    //     printf("无法获取接口信息！\n");
    // }

    _ifname_T ifnameInfo = {
        .get_all_ifname = get_all_interfaces,
        .interfaces_dump = interfaces_dump
    };

    InterfaceInfo *interfaces = ifnameInfo.get_all_ifname(&count);
    ifnameInfo.interfaces_dump(interfaces, count);

    return 0;
}