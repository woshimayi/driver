/*
 * @FilePath: /network/ifname_status/get_ifname_ip.c
 * @version:
 * @Author: sueRimn
 * @Date: 2021-01-22 10:35:10
 * @LastEditors: dof
 * @LastEditTime: 2023-06-13 14:52:15
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


void mactostr(char * strmac, char *dstmac)
{
    
}

int main(int argc, char const *argv[])
{
    char ipaddr[20] = {0};
    get_ip_by_ifname("veip0.2", ipaddr, sizeof(ipaddr));
    printf("ipaddr = %s\n", ipaddr);

    char macaddr[20] = {0};
    get_mac_by_ifname("veip0.2", macaddr, sizeof(macaddr));
    printf("macaddr = %s\n", macaddr);

    /* code */
    return 0;
}