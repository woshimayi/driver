/*
 * @FilePath: /network/ifname_status/get_ifname_ip.c
 * @version: 
 * @Author: sueRimn
 * @Date: 2021-01-22 10:35:10
 * @LastEditors: dof
 * @LastEditTime: 2021-12-22 11:35:44
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

int get_ip_by_ifname(char *ifname, char *pszIPaddr)
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
    sprintf(pszIPaddr, "%s", inet_ntoa(sin.sin_addr));
    printf("%s ip is %s \n", ifname, pszIPaddr);
    close(sockfd);
    return 0;
}


int main(int argc, char const *argv[])
{
    char ipaddr[20] = {0};
    get_ip_by_ifname("enp1s0", ipaddr);
    printf("ipaddr = %s\n", ipaddr);
    /* code */
    return 0;
}