/*
 * @*************************************: 
 * @FilePath: /network/ifname_status/get_gatway_by_ipandmask.c
 * @version: 
 * @Author: dof
 * @Date: 2023-12-19 20:09:02
 * @LastEditors: dof
 * @LastEditTime: 2023-12-19 20:12:14
 * @Descripttion: 
 * @**************************************: 
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netinet/if_ether.h>
#include <sys/ioctl.h>
#include <net/if.h>

#define MAX_ROUTE_INFO 10
#define SIZE 50

#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <sys/socket.h>

int main() {
    int fd;
    struct ifreq ifr;
    struct sockaddr_in *sin;
    char* iface = "eth0"; // 更改为您的网络接口名称

    fd = socket(AF_INET, SOCK_DGRAM, 0);

    ifr.ifr_addr.sa_family = AF_INET;
    strncpy(ifr.ifr_name , iface , IFNAMSIZ-1);
    ioctl(fd, SIOCGIFADDR, &ifr);

    sin = (struct sockaddr_in *)&ifr.ifr_addr;

    printf("IP地址: %s\n", inet_ntoa(sin->sin_addr));

    ioctl(fd, SIOCGIFNETMASK, &ifr);
    sin = (struct sockaddr_in *)&ifr.ifr_netmask;
    printf("子网掩码: %s\n", inet_ntoa(sin->sin_addr));

    close(fd);
    
    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));
    FILE* route = popen("ip route | grep default | awk '{print $3}'", "r");
    if (route == NULL) {
        perror("popen");
        exit(1);
    }
    fgets(buffer, sizeof(buffer), route);
    pclose(route);
    
    printf("网关地址: %s", buffer);

    return 0;
}

// int main(int argc, char const *argv[])
// {
//     get_gatway();
//     return 0;
// }


