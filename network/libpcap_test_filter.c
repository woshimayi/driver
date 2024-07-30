/*
 * @*************************************:
 * @FilePath: /network/libpcap_test_filter.c
 * @version:
 * @Author: dof
 * @Date: 2024-07-26 11:18:14
 * @LastEditors: dof
 * @LastEditTime: 2024-07-30 15:50:06
 * @Descripttion: complie: gcc libpcap_test_filter.c -lpcap -L/usr/lib/x86_64-linux-gnu
 * @**************************************:
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pcap.h>
#include <arpa/inet.h>
// #include <rpc/types.h>
#include <netinet/ip.h>
#include <netinet/ether.h>
#include <ctype.h>

#define PP(fmt, args...) printf("\033[0;32;31m[mdm :%s(%d)] " fmt "\033[1;37m\r\n", __func__, __LINE__, ##args)

struct ethheader
{
    u_char ether_dhost[6];
    // 目的mac地址
    u_char ether_shost[6];
    // 源mac地址
    u_short ether_type;
};

/* 802.1Q */
struct vlan_header
{
    unsigned short tci;
    unsigned short type;
};

/**
 * ppp header
 */
struct ppp_header
{
    unsigned short ppp_ver : 4,
        ppp_type : 4;
    unsigned char code;
    unsigned short sessionId;
    unsigned short payload;
    unsigned short nexttype;
};

/* IP Header */
struct ipheader
{
    unsigned char iph_ihl : 4,       // IP header length
        iph_ver : 4;                 // IP version
    unsigned char iph_tos;           // Type of service
    unsigned short int iph_len;      // IP Packet length (data + header)
    unsigned short int iph_ident;    // Identification
    unsigned short int iph_flag : 3, // Fragmentation flags
        iph_offset : 13;             // Flags offset
    unsigned char iph_ttl;           // Time to Live
    unsigned char iph_protocol;      // Protocol type
    unsigned short int iph_chksum;   // IP datagram checksum
    struct in_addr iph_sourceip;     // Source IP address
    struct in_addr iph_destip;       // Destination IP address
};

struct tcphdr
{
    unsigned short sport;    // 源端口
    unsigned short dport;    // 目标端口
    unsigned int seq;        // 序列号
    unsigned int ack_seq;    // 确认号
    unsigned char len;       // 首部长度
    unsigned char flag;      // 标志位
    unsigned short win;      // 窗口大小
    unsigned short checksum; // 校验和
    unsigned short urg;      // 紧急指针
};

/* UDP Header */
struct udphdr
{
    u_int16_t sport; /* source port */
    u_int16_t dport; /* destination port */
    u_int16_t ulen;  /* udp length */
    u_int16_t sum;   /* udp checksum */
};

// 处理数据包的回调函数
void handler(u_char *, const struct pcap_pkthdr *, const u_char *);
// 输出数据包payload
void print_data(unsigned char *, unsigned int);

/**
 * @brief
 *
 * @param argc
 * @param argv
 * @return int
 */
int main(int argc, char *argv[])
{
    pcap_t *handle;
    char errbuf[PCAP_ERRBUF_SIZE];
    struct bpf_program fp;
    char filter_exp[] = "";
    bpf_u_int32 net;
    // 统计数据包个数
    int packet_count = 0;
    pcap_if_t *devs;
    pcap_if_t *dev;
    // 可用的的设备
    int ret = pcap_findalldevs(&devs, errbuf);
    if (ret == -1)
    {
        printf("no dev up err %s\n", errbuf);
        return 0;
    }
    // 输出设备信息
    for (dev = devs; dev != NULL; dev = dev->next)
    {
        printf("Device:      %s\n", dev->name);
        printf("Description: %s\n", dev->description);
        printf("--------------------------------------\n");
    }
    // 选择默认设备
    dev = devs;

    printf("success Device:      %s\n", dev->name);

    // 打开设备
    handle = pcap_open_live("eth0", BUFSIZ, 1, 1000, errbuf);
    printf("listening on network card, ret: %p...\n", handle);
    // 编译规则
    printf("try to compile filter...\n");
    pcap_compile(handle, &fp, filter_exp, 0, net);
    printf("try to set filter...\n");
    pcap_setfilter(handle, &fp);

    // 开始捕获
    printf("start to sniff...\n");
    // 传递packet_count统计包个数
    pcap_loop(handle, -1, handler, (unsigned char *)&packet_count);
    // 关闭设备
    pcap_close(handle);
    pcap_freealldevs(devs);
    return 0;
}

/**
 * @brief 抓包回调函数
 *
 * @param args
 * @param hdr
 * @param packet
 */
void handler(u_char *args, const struct pcap_pkthdr *hdr, const u_char *packet)
{
    // 统计数据包
    int *packet_count = (int *)args;
    unsigned short next_type = 0;
    unsigned short next_proto = 0;
    (*packet_count)++;

    printf("\n\n***********************receive a packet***********************\n");
    printf("receive a packet, packet count: %d\n", *packet_count);

    // 获取以太网帧
    struct ethheader *eth = (struct ethheader *)packet;
    // 输出源和目的mac地址
    printf("Source MAC: ");
    for (int i = 0; i < 6; i++)
    {
        printf("%s%02x", i ? ":" : "", eth->ether_shost[i]);
    }
    printf("\t");
    printf("Destination MAC: ");
    for (int i = 0; i < 6; i++)
    {
        printf("%s%02x", i ? ":" : "", eth->ether_dhost[i]);
    }
    printf("\n");

    // 输出以太网类型
    printf("Ethernet Type: %04x\n", ntohs(eth->ether_type));
    next_type = ntohs(eth->ether_type);
    packet = packet + sizeof(struct ethheader);

    // 判断以太网类型 加 vlan 类型
    switch (next_type)
    {
    case 0x8100:
        printf("802.1Q\n");
        struct vlan_header *vlan = (struct vlan_header *)packet;
        print_data((unsigned char *)vlan, 2);
        printf("priority:   %d\n", (ntohs(vlan->tci) & 0xe000) >> 13);
        printf("dei:        %d\n", (ntohs(vlan->tci) & 0x1000) >> 12);
        printf("vlan:       %d\n", ntohs(vlan->tci) & 0xfff);
        printf("type:     %.4x\n", ntohs(vlan->type));
        next_type = ntohs(vlan->type);
        packet = packet + sizeof(struct vlan_header);
        break;
    default:
        PP("next_type = %04x", next_type);
        break;
    }

    switch (next_type)
    {
    case 0x8863:
    case 0x8864:
    {
        printf("ether type: PPPOE %d\n", sizeof(struct ppp_header));
        struct ppp_header *ppp = (struct ppp_header *)packet;
        print_data((unsigned char *)ppp, sizeof(struct ppp_header));
        printf("ver:      %d\n", ppp->ppp_ver);
        printf("type:     %d\n", ppp->ppp_type);
        printf("code:     %d\n", ppp->code);
        printf("sessid: %04x\n", ntohs(ppp->sessionId));
        printf("payload:  %d\n", ntohs(ppp->payload));
        printf("nexttype:%04x\n", ntohs(ppp->nexttype));
        next_type = ntohs(ppp->nexttype);
        packet = packet + sizeof(struct ppp_header);
        break;
    }
    default:
        PP("next_type = %04x", next_type);
        break;
    }

    switch (next_type)
    {
    case 0x0021:
        next_type = 0x0800;
        break;
    case 0xc021:
        break;
    default:
        break;
    }

    PP("next_type = %04x", next_type);
    // 判断以太网类型
    switch (next_type)
    {
    case 0x0800:
        printf("IPV4");
        struct ipheader *ip = (struct ipheader *)packet;
        printf("From: %s\t", inet_ntoa(ip->iph_sourceip));
        printf("To: %s\n", inet_ntoa(ip->iph_destip));
        next_proto = ip->iph_protocol;
        packet = packet + sizeof(struct ipheader);
        break;
    case 0x0806:
        printf("ether type: ARP\n");
        break;
    case 0x86DD:
        printf("ether type: IPV6\n");
        break;
    default:
        PP("next_type = %04x", next_type);
        break;
    }

    int payload_length;
    // 对上层协议处理
    switch (next_proto)
    {
    case IPPROTO_TCP:
        printf("Protocol: TCP\n");
        // 获取tcp报头
        struct tcphdr *tcp = (struct tcphdr *)packet;
        // 输出源和目的端口号
        printf("From: %d\t", ntohs(tcp->sport));
        printf("To: %d\n", ntohs(tcp->dport));
        // 输出协议的payload
        // 获取 TCP 数据的指针
        unsigned char *tcp_data = (unsigned char *)(tcp) + (tcp->len * 4);
        payload_length = hdr->len;
        // 输出 TCP 协议数据的十六进制和 ASCII 形式
        // print_data(tcp_data, payload_length);
        return;
    case IPPROTO_UDP:
        printf("Protocol: UDP\n");
        // 获取udp报头
        struct udphdr *udp = (struct udphdr *)(packet + sizeof(struct ethheader) + sizeof(struct iphdr));
        // 输出源和目的端口号
        printf("From: %d\t", ntohs(udp->sport));
        printf("To: %d\n", ntohs(udp->dport));
        // 输出协议的payload
        // 获取 UDP 数据的指针
        unsigned char *udp_data = (unsigned char *)(udp) + sizeof(struct udphdr);
        // 计算 UDP payload 的长度
        payload_length = hdr->len;
        // 输出 UDP 协议数据的十六进制和 ASCII 形式
        // print_data(udp_data, payload_length);
        return;
    case IPPROTO_ICMP:
        printf("Protocol: ICMP\n");
        return;
    default:
        PP("Protocol: others %d\n", next_proto);
        return;
    }
}

/**
 * @brief 抓包打印字节
 *
 * @param data
 * @param len
 */
void print_data(unsigned char *data, unsigned int len)
{
    int i;
    printf("receve %d bytes\n***************************payload****************************\n", len);
    for (i = 0; i < len; i++)
    {
        printf("%02X", data[i]); // 输出十六进制值，并在值之间添加空格
        if ((i + 1) % 16 == 0)
        {
            printf("\t");
            for (int j = i - 15; j <= i; j++)
            {
                if (data[j] >= 32 && data[j] <= 126)
                {
                    printf("%c", data[j]); // 输出 ASCII 字符（如果可打印）
                }
                else
                {
                    printf(".");
                }
                if ((j + 1) % 16 == 0)
                {
                    printf("\n"); // 在每行的末尾打印换行符
                }
            }
        }
    }
    printf("\n");
}