// test_custom_protocol.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <net/if.h>

#define CUSTOM_PROTOCOL 0x8888
#define CUSTOM_HEADER_SIZE 20

struct custom_header {
    uint32_t src_addr;    // 源地址
    uint32_t dst_addr;    // 目的地址
    uint16_t seq_num;     // 序列号
    uint16_t msg_type;    // 消息类型
    uint32_t data_len;    // 数据长度
};

int main() {
    int sockfd;
    struct sockaddr_ll dest_addr;
    struct custom_header custom_hdr;
    unsigned char packet[ETH_FRAME_LEN];
    
    // 创建原始套接字
    sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (sockfd < 0) {
        perror("socket creation failed");
        exit(1);
    }
    
    // 设置目标地址
    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.sll_family = AF_PACKET;
    dest_addr.sll_protocol = htons(CUSTOM_PROTOCOL);
    dest_addr.sll_ifindex = if_nametoindex("eth0"); // 替换为您的接口名
    dest_addr.sll_halen = ETH_ALEN;
    
    // 设置目标MAC地址（替换为实际的目标MAC）
    unsigned char dest_mac[6] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55};
    memcpy(dest_addr.sll_addr, dest_mac, ETH_ALEN);
    
    // 构建以太网帧
    memset(packet, 0, ETH_FRAME_LEN);
    
    // 以太网头部
    struct ethhdr *eth = (struct ethhdr*)packet;
    memcpy(eth->h_dest, dest_mac, ETH_ALEN);
    memcpy(eth->h_source, dest_mac, ETH_ALEN); // 临时使用相同MAC
    eth->h_proto = htons(CUSTOM_PROTOCOL);
    
    // 自定义协议头部
    struct custom_header *hdr = (struct custom_header*)(packet + sizeof(struct ethhdr));
    hdr->src_addr = inet_addr("127.0.0.1");
    hdr->dst_addr = inet_addr("127.0.0.1");
    hdr->seq_num = htons(1);
    hdr->msg_type = htons(0x0001); // 心跳包
    hdr->data_len = htonl(0);
    
    // 发送数据包
    int packet_len = sizeof(struct ethhdr) + CUSTOM_HEADER_SIZE;
    if (sendto(sockfd, packet, packet_len, 0, 
               (struct sockaddr*)&dest_addr, sizeof(dest_addr)) < 0) {
        perror("sendto failed");
    } else {
        printf("Custom protocol packet sent successfully\n");
    }
    
    close(sockfd);
    return 0;
}