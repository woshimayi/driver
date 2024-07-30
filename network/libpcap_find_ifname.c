/*
 * @*************************************:
 * @FilePath: /network/libpcap_find_ifname.c
 * @version:
 * @Author: dof
 * @Date: 2024-07-25 19:48:08
 * @LastEditors: dof
 * @LastEditTime: 2024-07-26 11:06:06
 * @Descripttion: complie: gcc libpcap_find_ifname.c -lpcap -L/usr/lib/x86_64-linux-gnu
 * @**************************************:
 */

#if 0
#include <stdio.h>
#include <stdlib.h>
#include <pcap.h>

int main()
{
    pcap_if_t *alldevs;
    char errbuf[PCAP_ERRBUF_SIZE];

    /* 获取所有网络设备 */
    if (pcap_findalldevs(&alldevs, errbuf) == -1)
    {
        fprintf(stderr, "Error in pcap_findalldevs: %s\n", errbuf);
        exit(1);
    }

    /* 遍历链表 */
    pcap_if_t *d;
    for (d = alldevs; d; d = d->next)
    {
        printf("%s: ", d->name);
        if (d->description)
            printf("%s\n", d->description);
        else
            printf("No description available\n");

        struct pcap_addr *a;
        // 遍历接口的地址列表
        for (a = d->addresses; a; a = a->next)
        {
            switch (a->addr->sa_family)
            {
            case AF_INET:
                printf("\tIP: %s\n", inet_ntoa(((struct sockaddr_in *)a->addr)->sin_addr));
                break;
            case AF_INET6:
                // 处理 IPv6 地址
                break;
            default:
                break;
            }
        }
    }

    /* 释放内存 */
    pcap_freealldevs(alldevs);

    return 0;
}
#else
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pcap.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>

void print_device_info(char *dev)
{
    pcap_if_t *alldevs, *d;
    char errbuf[PCAP_ERRBUF_SIZE];
    pcap_t *p;

    // 获取所有设备列表
    if (pcap_findalldevs(&alldevs, errbuf) == -1)
    {
        fprintf(stderr, "Error in pcap_findalldevs: %s\n", errbuf);
        exit(1);
    }

    for (d = alldevs; d; d = d->next)
    {
        // if (strcmp(d->name, dev) == 0)
        {
            // 打开设备
            p = pcap_open_live(d->name, BUFSIZ, 1, 1000, errbuf);
            if (p == NULL)
            {
                fprintf(stderr, "Error opening device %s: %s\n", d->name, errbuf);
                continue;
            }

            // 获取设备统计信息
            struct pcap_stat stats;
            if (pcap_stats(p, &stats) < 0)
            {
                fprintf(stderr, "Error getting stats: %s\n", pcap_geterr(p));
                continue;
            }

            printf("%d\n", stats.ps_recv);		/* number of packets received */
            printf("%d\n", stats.ps_drop);		/* number of packets dropped */
            printf("%d\n", stats.ps_ifdrop);	/* drops by interface -- only supported on some platforms */


            // 打印设备信息（这里仅展示部分信息，可根据需要扩展）
            printf("Device: %s\n", d->name);
            // ... (使用 ifconfig 或其他工具获取IP、子网掩码、网关、MAC)
            pcap_close(p);
            break;
        }
    }

    pcap_freealldevs(alldevs);
}

int main()
{
    char *dev = "eth0"; // 指定要查询的设备
    print_device_info(dev);
    return 0;
}

#endif