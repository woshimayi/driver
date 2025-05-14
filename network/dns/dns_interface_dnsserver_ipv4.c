#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <netinet/udp.h>
#include <errno.h>

// DNS头部结构
struct dns_header
{
    unsigned short id;        // 标识
    unsigned char rd : 1;     // 递归期望
    unsigned char tc : 1;     // 截断
    unsigned char aa : 1;     // 授权回答
    unsigned char opcode : 4; // 操作码
    unsigned char qr : 1;     // 查询/响应标志
    unsigned char rcode : 4;  // 响应码
    unsigned char cd : 1;     // 禁用检查
    unsigned char ad : 1;     // 认证数据
    unsigned char z : 1;      // 保留
    unsigned char ra : 1;     // 递归可用
    unsigned short qdcount;   // 问题数
    unsigned short ancount;   // 回答数
    unsigned short nscount;   // 授权记录数
    unsigned short arcount;   // 附加记录数
};

// 创建DNS查询报文
int create_dns_query(const char *domain, unsigned char *buffer, int buffer_size)
{
    if (domain == NULL || buffer == NULL || buffer_size < 512)
    {
        return -1;
    }

    // 填充DNS头部
    struct dns_header *header = (struct dns_header *)buffer;
    header->id = htons(0x1234); // 任意ID
    header->qr = 0;             // 查询
    header->opcode = 0;         // 标准查询
    header->aa = 0;
    header->tc = 0;
    header->rd = 1; // 期望递归
    header->ra = 0;
    header->z = 0;
    header->ad = 0;
    header->cd = 0;
    header->rcode = 0;
    header->qdcount = htons(1); // 一个问题
    header->ancount = 0;
    header->nscount = 0;
    header->arcount = 0;

    // 填充查询问题
    unsigned char *query = buffer + sizeof(struct dns_header);
    const char *dot;
    char *label = (char *)domain;
    int pos = 0;

    while ((dot = strchr(label, '.')) != NULL)
    {
        int len = dot - label;
        query[pos++] = len;
        memcpy(query + pos, label, len);
        pos += len;
        label = dot + 1;
    }

    // 最后一个标签
    int len = strlen(label);
    query[pos++] = len;
    memcpy(query + pos, label, len);
    pos += len;

    // 结束标记和查询类型、类
    query[pos++] = 0; // 结束标记
    query[pos++] = 0;
    query[pos++] = 1; // QTYPE=A (主机地址)
    query[pos++] = 0;
    query[pos++] = 1; // QCLASS=IN (Internet)

    return sizeof(struct dns_header) + pos;
}

// 设置socket绑定到特定接口
int bind_to_interface(int sockfd, const char *ifname)
{
    if (ifname == NULL)
    {
        return 0; // 不绑定特定接口
    }

    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, ifname, IFNAMSIZ - 1);

    if (setsockopt(sockfd, SOL_SOCKET, SO_BINDTODEVICE, (void *)&ifr, sizeof(ifr)))
    {
        perror("setsockopt(SO_BINDTODEVICE) failed");
        return -1;
    }

    return 0;
}

// 解析DNS响应
void parse_dns_response(unsigned char *buffer, int len)
{
    if (len < sizeof(struct dns_header))
    {
        fprintf(stderr, "Invalid DNS response length\n");
        return;
    }

    struct dns_header *header = (struct dns_header *)buffer;
    printf("DNS Response:\n");
    printf("  ID: 0x%04x\n", ntohs(header->id));
    printf("  QR: %d (%s)\n", header->qr, header->qr ? "Response" : "Query");
    printf("  Opcode: %d\n", header->opcode);
    printf("  AA: %d\n", header->aa);
    printf("  TC: %d\n", header->tc);
    printf("  RD: %d\n", header->rd);
    printf("  RA: %d\n", header->ra);
    printf("  RCODE: %d\n", header->rcode);
    printf("  QDCOUNT: %d\n", ntohs(header->qdcount));
    printf("  ANCOUNT: %d\n", ntohs(header->ancount));
    printf("  NSCOUNT: %d\n", ntohs(header->nscount));
    printf("  ARCOUNT: %d\n", ntohs(header->arcount));

    // 跳过问题部分
    unsigned char *ptr = buffer + sizeof(struct dns_header);
    int i;
    for (i = 0; i < ntohs(header->qdcount); i++)
    {
        while (*ptr != 0)
        {
            if ((*ptr & 0xC0) == 0xC0)
            { // 压缩指针
                ptr += 2;
                break;
            }
            ptr += *ptr + 1;
        }
        ptr += 5; // 跳过类型和类
    }

    // 解析回答部分
    for (i = 0; i < ntohs(header->ancount); i++)
    {
        if ((ptr[0] & 0xC0) == 0xC0)
        { // 压缩指针
            ptr += 2;
        }
        else
        {
            while (*ptr != 0)
            {
                ptr += *ptr + 1;
            }
            ptr += 1;
        }

        unsigned short type = ntohs(*(unsigned short *)ptr);
        ptr += 2;
        ptr += 2; // 跳过类
        ptr += 4; // 跳过TTL
        unsigned short rdlength = ntohs(*(unsigned short *)ptr);
        ptr += 2;

        if (type == 1 && rdlength == 4)
        { // A记录
            struct in_addr addr;
            memcpy(&addr, ptr, 4);
            printf("  A Record: %s\n", inet_ntoa(addr));
        }
        else if (type == 5)
        { // CNAME记录
            printf("  CNAME Record: ");
            unsigned char *name_ptr = ptr;
            while (*name_ptr != 0)
            {
                if ((*name_ptr & 0xC0) == 0xC0)
                { // 压缩指针
                    unsigned short offset = ntohs(*(unsigned short *)name_ptr) & 0x3FFF;
                    name_ptr = buffer + offset;
                }
                else
                {
                    int len = *name_ptr;
                    name_ptr++;
                    fwrite(name_ptr, 1, len, stdout);
                    name_ptr += len;
                    if (*name_ptr != 0)
                    {
                        printf(".");
                    }
                }
            }
            printf("\n");
        }
        ptr += rdlength;
    }
}

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        fprintf(stderr, "Usage: %s <domain> <dns_server> [interface]\n", argv[0]);
        fprintf(stderr, "Example: %s example.com 8.8.8.8 eth0\n", argv[0]);
        return 1;
    }

    const char *domain = argv[1];
    const char *dns_server = argv[2];
    const char *interface = argc > 3 ? argv[3] : NULL;

    // 创建UDP socket
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        perror("socket creation failed");
        return 1;
    }

    // 绑定到指定接口
    if (bind_to_interface(sockfd, interface) != 0)
    {
        close(sockfd);
        return 1;
    }

    // 设置超时
    struct timeval timeout;
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)))
    {
        perror("setsockopt(SO_RCVTIMEO) failed");
        close(sockfd);
        return 1;
    }

    // 准备DNS服务器地址
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(53); // DNS端口
    if (inet_pton(AF_INET, dns_server, &servaddr.sin_addr) <= 0)
    {
        perror("invalid DNS server address");
        close(sockfd);
        return 1;
    }

    // 创建DNS查询
    unsigned char sendbuf[512], recvbuf[512];
    int query_len = create_dns_query(domain, sendbuf, sizeof(sendbuf));
    if (query_len < 0)
    {
        fprintf(stderr, "Failed to create DNS query\n");
        close(sockfd);
        return 1;
    }

    // 发送DNS查询
    printf("Sending DNS query for %s to %s\n", domain, dns_server);
    if (interface)
    {
        printf("Using network interface: %s\n", interface);
    }

    ssize_t sent = sendto(sockfd, sendbuf, query_len, 0,
                          (const struct sockaddr *)&servaddr, sizeof(servaddr));
    if (sent != query_len)
    {
        perror("sendto failed");
        close(sockfd);
        return 1;
    }

    // 接收DNS响应
    struct sockaddr_in from_addr;
    socklen_t from_len = sizeof(from_addr);
    ssize_t recv_len = recvfrom(sockfd, recvbuf, sizeof(recvbuf), 0,
                                (struct sockaddr *)&from_addr, &from_len);
    if (recv_len < 0)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
            fprintf(stderr, "DNS query timed out\n");
        }
        else
        {
            perror("recvfrom failed");
        }
        close(sockfd);
        return 1;
    }

    // 解析并打印响应
    parse_dns_response(recvbuf, recv_len);

    close(sockfd);
    return 0;
}