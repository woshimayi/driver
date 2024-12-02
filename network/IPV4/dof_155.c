/*
 * @*************************************:
 * @FilePath     : /network/IPV4/dof_155.c
 * @version      :
 * @Author       : dof
 * @Date         : 2024-11-26 17:23:42
 * @LastEditors  : dof
 * @LastEditTime : 2024-12-02 14:37:32
 * @Descripttion :  socket icmp
 * @compile      :
 * @**************************************:
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <netinet/ip_icmp.h>
#include <errno.h>

#define PACKET_SIZE 64
#define IPPROTO_DOF 155

#define PP(fmt, args...) printf("\033[0;32;31m[mdm :%s(%d)] " fmt "\033[1;37m\r\n", __func__, __LINE__, ##args)

// 计算校验和
unsigned short checksum(void *b, int len)
{
    unsigned short *buf = b;
    unsigned int sum = 0;
    unsigned short result;

    for (sum = 0; len > 1; len -= 2)
        sum += *buf++;
    if (len == 1)
        sum += *(unsigned char *)buf;

    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    result = ~sum;
    return result;
}

// 创建ICMP报文
void create_icmp_echo_request(struct icmphdr *icmp, int seq)
{
    icmp->type = ICMP_ECHO;       // 设置为Echo请求
    icmp->code = 0;               // 没有错误
    icmp->checksum = 0;           // 先置为0，计算校验和时需要
    icmp->un.echo.id = getpid();  // 设置为进程ID
    icmp->un.echo.sequence = seq; // 设置序列号

    icmp->checksum = checksum(icmp, sizeof(struct icmphdr)); // 计算校验和
}

int main(int argc, char *argv[])
{
#if 0
    // if (argc != 2) {
    //     printf("Usage: %s <IP address>\n", argv[0]);
    //     return -1;
    // }
    // const char *target_ip = argv[1];
#else
    const char *target_ip = "172.16.26.189";
#endif

    int sockfd;
    struct sockaddr_in target_addr;
    char packet[PACKET_SIZE];
    struct icmphdr *icmp_hdr = (struct icmphdr *)packet;
    struct iphdr *ip_hdr = (struct iphdr *)packet;

    sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_DOF);
    if (sockfd < 0)
    {
        perror("Socket creation failed");
        return -1;
    }

    target_addr.sin_family = AF_INET;
    target_addr.sin_port = 0;
    target_addr.sin_addr.s_addr = inet_addr(target_ip);

    // 创建ICMP请求
    // create_icmp_echo_request(icmp_hdr, 1); // 序列号从1开始

    FILE *fd = fopen("IPV4/dof_test.c", "rb");
    if (NULL == fd)
    {
        perror("");
        return -1;
    }

    int c = 0;
    while ((c = fread(packet, 1, sizeof(packet), fd)) > 0)
    // while(fgets(packet, sizeof(packet), fd) != NULL)
    {
        PP("%s", packet);
        int nsend = sendto(sockfd, packet, c, 0,
                           (struct sockaddr *)&target_addr, sizeof(target_addr));
        if (nsend < 0)
        {
            perror("Sendto failed");
            close(sockfd);
            return -1;
        }
        memset(packet, 0, sizeof(packet));
    }

    printf("Sent ICMP Echo Request to %s\n", target_ip);

    // 接收响应
    struct sockaddr_in from;
    socklen_t fromlen = sizeof(from);
    int nrecv = recvfrom(sockfd, packet, sizeof(packet), 0, (struct sockaddr *)&from, &fromlen);
    if (nrecv < 0)
    {
        perror("Recvfrom failed");
        close(sockfd);
        return -1;
    }

    ip_hdr = (struct iphdr *)packet;
    icmp_hdr = (struct icmphdr *)(packet + (ip_hdr->ihl << 2));

    // 打印接收到的响应信息
    printf("Received ICMP Echo Reply from %s\n", inet_ntoa(from.sin_addr));
    printf("ICMP Type: %d, Code: %d, ID: %d, Sequence: %d\n",
           icmp_hdr->type, icmp_hdr->code, icmp_hdr->un.echo.id, icmp_hdr->un.echo.sequence);

    close(sockfd);
    return 0;
}