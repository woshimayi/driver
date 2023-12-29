/*
 * @*************************************: 
 * @FilePath: /network/ifname_status/get_ifname_ip.c
 * @version: 
 * @Author: dof
 * @Date: 2023-08-20 17:08:35
 * @LastEditors: dof
 * @LastEditTime: 2023-12-22 15:38:51
 * @Descripttion: 
 * @**************************************: 
 */
/*
 * @FilePath: /network/ifname_status/get_ifname_ip.c
 * @version:
 * @Author: sueRimn
 * @Date: 2021-01-22 10:35:10
 * @LastEditors: dof
 * @LastEditTime: 2023-12-21 18:02:35
 * @Descripttion: 获取 指定接口地址
 */

#include <stdio.h>
#include <string.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

int get_ip_by_ifname(char *ifname, char *pszIPaddr, unsigned int len)
{
    int sockfd;
    struct ifreq ifr;
    struct sockaddr_in sin;
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        return -1;
    }
    strcpy(ifr.ifr_name, ifname);
    if (ioctl(sockfd, SIOCGIFADDR, &ifr) < 0)
    {
        close(sockfd);
        return -1;
    }
    memcpy(&sin, &ifr.ifr_dstaddr, sizeof(sin));
    snprintf(pszIPaddr, len, "%s", inet_ntoa(sin.sin_addr));
    printf("%s ip is %s \n", ifname, pszIPaddr);


    /* 子网掩码 */
    if (ioctl(sockfd, SIOCGIFNETMASK, &ifr) < 0)
    {
        char str[256];
        sprintf(str, "SIOCGIFADDR ioctl %s\n", ifr.ifr_name);
        perror(str);
    }
    else
    {
        printf("子网掩码:%s\n",
                (char *)inet_ntoa(((struct sockaddr_in *)(&ifr.ifr_addr))->sin_addr));
    }

    close(sockfd);
    return 0;
}




int get_mac_by_ifname(char *ifname, char *pszMac, unsigned int len)
{
    int sockfd;
    struct ifreq ifr;
    struct sockaddr_in sin;
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        return -1;
    }
    strcpy(ifr.ifr_name, ifname);
    if (0 > (ioctl(sockfd, SIOCGIFHWADDR, (char *)&ifr)))
    {
        close(sockfd);
        return -1;
    }

    printf("mac = %s\n", ifr.ifr_hwaddr.sa_data);
    snprintf(pszMac, len, "%02x%02x%02x%02x%02x%02x\n\n",
             (unsigned char)ifr.ifr_hwaddr.sa_data[0],
             (unsigned char)ifr.ifr_hwaddr.sa_data[1],
             (unsigned char)ifr.ifr_hwaddr.sa_data[2],
             (unsigned char)ifr.ifr_hwaddr.sa_data[3],
             (unsigned char)ifr.ifr_hwaddr.sa_data[4],
             (unsigned char)ifr.ifr_hwaddr.sa_data[5]);
    printf("%s mac is %s \n", ifname, pszMac);
    close(sockfd);
    return 0;
}


// void get_ipv6()
// {
//     if ((f = fopen("/proc/net/if_inet6", "r")) != NULL)
// {
//     while (fscanf(f, "%4s%4s%4s%4s%4s%4s%4s%4s %02x %02x %02x %02x %20s\n", addr6p[0], addr6p[1], addr6p[2], addr6p[3], addr6p[4], addr6p[5], addr6p[6], addr6p[7], &if_idx, &plen, &scope, &dad_status, devname) != EOF)
//     {
//         if (!strcmp("eth0", ptr->name))
//         {
//             sprintf(addr6, "%s:%s:%s:%s:%s:%s:%s:%s", addr6p[0], addr6p[1], addr6p[2], addr6p[3], addr6p[4], addr6p[5], addr6p[6], addr6p[7]);
//             inet6_aftype.input(1, addr6, (struct sockaddr *)&sap);
//             printf(_(" inet6 addr: %s/%d"), inet6_aftype.sprint((struct sockaddr *)&sap, 1), plen);
//             printf(_(" Scope:"));
//             switch (scope)
//             {
//             case 0:
//                 printf(_("Global"));
//                 break;
//             case IPV6_ADDR_LINKLOCAL:
//                 printf(_("Link"));
//                 break;
//             case IPV6_ADDR_SITELOCAL:
//                 printf(_("Site"));
//                 break;
//             case IPV6_ADDR_COMPATv4:
//                 printf(_("Compat"));
//                 break;
//             case IPV6_ADDR_LOOPBACK:
//                 printf(_("Host"));
//                 break;
//             default:
//                 printf(_("Unknown"));
//             }
//             printf("\n");
//         }
//     }
//     fclose(f);
// }
// }

int main(int argc, char const *argv[])
{
    char ipaddr[20] = {0};
    get_ip_by_ifname("eth0", ipaddr, sizeof(ipaddr));
    printf("ipaddr = %s\n", ipaddr);

    char macaddr[20] = {0};
    get_mac_by_ifname("eth0", macaddr, sizeof(macaddr));
    printf("macaddr = %s\n", macaddr);

    /* code */
    return 0;
}

