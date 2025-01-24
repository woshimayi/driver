/*
 * @*************************************:
 * @FilePath     : /network/net_util/get_mac_by_ip.c
 * @version      :
 * @Author       : dof
 * @Date         : 2025-01-06 15:53:31
 * @LastEditors  : dof
 * @LastEditTime : 2025-01-06 19:10:32
 * @Descripttion : by arp package get mac ip info
 * @compile      :
 * @**************************************:
 */

/**
 * @send_arp.c
 * @This software is intended to be used as a example to show how to send and receive arp request with Linux * PF_PACKET interface
 * @Author：xuelei
 **/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <sys/types.h>
#include <asm/types.h>
#include <features.h> /* 需要里面的 glibc 版本号 */
#if __GLIBC__ >= 2 && __GLIBC_MINOR >= 1
#include <netpacket/packet.h>
#include <net/ethernet.h> /* 链路层（L2）协议 */
#else
#include <asm/types.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h> /* 链路层协议 */
#endif
#include <netinet/if_ether.h>

#define INLEN 4
#define MAC_BCAST_ADDR (uint8_t *)"\xff\xff\xff\xff\xff\xff"

void usage_quit(char *arg0);
int get_ifi(char *dev, char *mac, int macln, struct in_addr *lc_addr, int ipln);
void prmac(u_char *ptr);

int getmac_byip(char *ip)
{
    int fd, salen, n;
    u_char *mac;
    char recv_buf[120], rep_addr[16];
    struct in_addr lc_addr, req_addr;
    struct sockaddr_ll reqsa, repsa;
    struct arp_pkt
    {
        struct ether_header eh;
        struct ether_arp ea;
        u_char padding[18];
    } req;

    bzero(&reqsa, sizeof(reqsa));
    reqsa.sll_family = PF_PACKET;
    reqsa.sll_ifindex = if_nametoindex("eth0");

    if ((fd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ARP))) < 0)
    {
        perror("Socket error");
        exit(1);
    }

    mac = (char *)malloc(ETH_ALEN);
    bzero(&req, sizeof(req));

    if (get_ifi("eth0", mac, ETH_ALEN, &lc_addr, INLEN))
    {
        fprintf(stderr, "Error: Get host’s information failed\n");
        return 0;
    }

    printf("        Host MAC is: %02x:%02x:%02x:%02x:%02x:%02x\n", *mac, *(mac + 1), *(mac + 2), *(mac + 3), *(mac + 4), *(mac + 5));
    inet_ntop(AF_INET, &lc_addr, rep_addr, 1024);
    // printf("       Host IP is: %s\n", inet_ntop(AF_INET, &lc_addr, rep_addr, 1024));
    printf("       Host IP is: %s\n", rep_addr);
    /* 填写以太网头部*/
    memcpy(req.eh.ether_dhost, MAC_BCAST_ADDR, ETH_ALEN);
    memcpy(req.eh.ether_shost, mac, ETH_ALEN);
    req.eh.ether_type = htons(ETHERTYPE_ARP);

    /* 填写arp数据 */
    req.ea.arp_hrd = htons(ARPHRD_ETHER);
    req.ea.arp_pro = htons(ETHERTYPE_IP);
    req.ea.arp_hln = ETH_ALEN;
    req.ea.arp_pln = INLEN;
    req.ea.arp_op = htons(ARPOP_REQUEST);
    memcpy(req.ea.arp_sha, mac, ETH_ALEN);
    memcpy(req.ea.arp_spa, &lc_addr, INLEN);
    inet_aton(ip, req.ea.arp_tpa);

    bzero(recv_buf, sizeof(recv_buf));
    bzero(&repsa, sizeof(repsa));
    salen = sizeof(struct sockaddr_ll);

    if ((n = sendto(fd, &req, sizeof(req), 0, (struct sockaddr *)&reqsa, sizeof(reqsa))) <= 0)
    {
        perror("Sendto error");
        exit(1);
    }
    printf("Broadcast arp request of %s, %d bytes be sent\n\n", ip, n);

    while (1)
    {
        if ((n = recvfrom(fd, recv_buf, sizeof(req), 0, (struct sockaddr *)&repsa, &salen)) <= 0)
        {
            perror("Recvfrom error");
            exit(1);
        }
        if (ntohs(*(__be16 *)(recv_buf + 20)) == 2 && !memcmp(req.ea.arp_tpa, recv_buf + 28, 4))
        {
            printf("Response from %s, %d bytes received\n", ip, n);
            inet_ntop(AF_INET, (struct in_addr *)(recv_buf + 28), rep_addr, 1024);
            printf("        Peer IP is: %s\n", rep_addr);
            prmac((u_char *)(recv_buf + 22)); // prmac( (u_char *)(recv_buf + 6) );
            break;
        }
        else
        {
            printf("Have recive data\n");
            break;
        }
    }

    free(mac);
}

int get_ifi(char *dev, char *mac, int macln, struct in_addr *lc_addr, int ipln)
{
    int reqfd, n;
    struct ifreq macreq;

    reqfd = socket(AF_INET, SOCK_DGRAM, 0);
    strcpy(macreq.ifr_name, dev);

    /* 获取本地接口MAC地址*/
    if (ioctl(reqfd, SIOCGIFHWADDR, &macreq) != 0)
        return 1;
    memcpy(mac, macreq.ifr_hwaddr.sa_data, macln);

    /* 获取本地接口IP地址*/
    if (ioctl(reqfd, SIOCGIFADDR, &macreq) != 0)
        return 1;
    memcpy(lc_addr, &((struct sockaddr_in *)(&macreq.ifr_addr))->sin_addr, ipln);

    return 0;
}

void prmac(u_char *ptr)
{
    printf("        Peer MAC is: %02x:%02x:%02x:%02x:%02x:%02x\n", *ptr, *(ptr + 1), *(ptr + 2), *(ptr + 3), *(ptr + 4), *(ptr + 5));
}

void usage_quit(char *arg0)
{
    fprintf(stderr, "Usage: %s <query_IP>\n", arg0);
    exit(1);
}


int getarptable()
{
    int s;
    struct ifreq ifr;
    struct arpreq arpreq;

    // 创建 socket
    s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s < 0) {
        perror("socket");
        return 1;
    }

    // 遍历所有网络接口
    char *dev = "eth0"; // 从 "eth0" 开始，可以遍历所有接口
    while (*dev != '\0') {
        strncpy(ifr.ifr_name, dev, IFNAMSIZ);

        // 获取接口信息
        if (ioctl(s, SIOCGIFFLAGS, &ifr) == 0) {
            // 判断接口是否up
            if (ifr.ifr_flags & IFF_UP) {
                // 填充 arpreq 结构体
                memset(&arpreq, 0, sizeof(arpreq));
                strncpy(arpreq.arp_pa.sa_family, ARPHRD_ETHER, sizeof(arpreq.arp_pa.sa_family));

                // 获取 ARP 表信息
                if (ioctl(s, SIOCGARP, &arpreq) == 0) {
                    printf("IP: %s\tMAC: %02x:%02x:%02x:%02x:%02x:%02x\n",
                           inet_ntoa(((struct sockaddr_in *)&arpreq.arp_pa)->sin_addr),
                           arpreq.arp_ha.sa_data[0], arpreq.arp_ha.sa_data[1],
                           arpreq.arp_ha.sa_data[2], arpreq.arp_ha.sa_data[3],
                           arpreq.arp_ha.sa_data[4], arpreq.arp_ha.sa_data[5]);
                } else {
                    perror("ioctl SIOCGARP");
                }
            }
        }

        // 获取下一个接口名
        struct ifreq *ifr_next = (struct ifreq *)((char *)&ifr + IFNAMSIZ + sizeof(ifr.ifr_addr));
        dev = ifr_next->ifr_name;
    }

    close(s);
    return 0;
}

int main(int argc, char const *argv[])
{
    // getmac_byip("172.16.80.2");
    getarptable();
    return 0;
}
