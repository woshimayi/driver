#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>



#define MAX_WAIT_TIME   1
#define MAX_NO_PACKETS  1
#define ICMP_HEADSIZE 8
#define PACKET_SIZE     4096

//#define _USE_DNS

#define PP printf("%s %d\n", __FUNCTION__, __LINE__);

struct timeval tvsend, tvrecv;
struct sockaddr_in dest_addr, recv_addr;
int sockfd;
pid_t pid;
char sendpacket[PACKET_SIZE];
char recvpacket[PACKET_SIZE];

//函数定义
void timeout(int signo);
unsigned short cal_chksum(unsigned short *addr, int len);
int pack(int pkt_no, char *sendpacket);
int send_packet(int pkt_no, char *sendpacket);
int recv_packet(int pkt_no, char *recvpacket);
int unpack(int cur_seq, char *buf, int len);
void tv_sub(struct timeval *out, struct timeval *in);
void _CloseSocket();
int lookup(char *host, int *d);

int lookup(char *host, int *d)
{
    struct addrinfo hints;
    struct addrinfo *res;
    int ret;
    struct sockaddr_in *addr;
    char ipbuf[16];

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET; /* Allow IPv4 */
    hints.ai_flags = AI_PASSIVE; /* For wildcard IP address */
    hints.ai_protocol = 0; /* Any protocol */
    hints.ai_socktype = SOCK_STREAM;

    ret = getaddrinfo(host, NULL, &hints, &res);

    if (ret != 0)
    {
        perror("getaddrinfo");
        return ret;
    }

    struct addrinfo *cur;
    for (cur = res; cur != NULL; cur = cur->ai_next)
    {
        addr = (struct sockaddr_in *)cur->ai_addr;
        printf("%s\n", inet_ntop(AF_INET,
                                 &addr->sin_addr, ipbuf, 16));
    }
    freeaddrinfo(res);
    printf("ret = %d ipbuf = %s\n", ret, ipbuf);
    *d = inet_addr(ipbuf);

    return ret;
}


bool NetIsOk()
{
    double rtt;
    struct hostent *host;
    struct protoent *protocol;
    int i, recv_status, d;

#ifdef _USE_DNS //如果定义该宏，则可以使用域名进行判断网络连接，例如www.baidu.com  
    /* 设置目的地址信息 */
    char hostname[32];
    sprintf(hostname, "%s", "www.baidu.com");
    bzero(&dest_addr, sizeof(dest_addr));
    dest_addr.sin_family = AF_INET;
    PP
    if (lookup(hostname, &d) != 0)
    {
        printf("[NetStatus] 1error : Can't get serverhost info!\n");
        PP
        return false;
    }
    printf("%d\n", d);
    PP
    bcopy((char *)host->h_addr, (char *)&d, host->h_length);
    PP
#else //如果不使用域名，则只能用ip地址直接发送icmp包，例如谷歌的地址：8.8.8.8  
    dest_addr.sin_addr.s_addr = inet_addr("8.8.8.8");
#endif

    PP
    if ((sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) < 0)
    {
        /* 创建原始ICMP套接字 */
        PP
        printf("[NetStatus] 2error : socket\n");
        PP
        return false;
    }

    int iFlag;
    if (iFlag = fcntl(sockfd, F_GETFL, 0) < 0)
    {
        printf("[NetStatus] 3error : fcntl(sockfd,F_GETFL,0)\n");
        _CloseSocket();
        return false;
    }
    iFlag |= O_NONBLOCK;
    if (iFlag = fcntl(sockfd, F_SETFL, iFlag) < 0)
    {
        printf("[NetStatus] 4error : fcntl(sockfd,F_SETFL,iFlag )\n");
        _CloseSocket();
        return false;
    }

    pid = getpid();
    for (i = 0; i < MAX_NO_PACKETS; i++)
    {

        if (send_packet(i, sendpacket) < 0)
        {
            printf("[NetStatus] 5error : send_packet\n");
            _CloseSocket();
            return false;
        }

        if (recv_packet(i, recvpacket) > 0)
        {
            _CloseSocket();
            return true;
        }

    }
    _CloseSocket();
    return false;
}



int send_packet(int pkt_no, char *sendpacket)
{
    int packetsize;
    packetsize = pack(pkt_no, sendpacket);
    gettimeofday(&tvsend, NULL);
    if (sendto(sockfd, sendpacket, packetsize, 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr)) < 0)
    {
        printf("[NetStatus] 6error : sendto error\n");
        return -1;
    }
    return 1;
}


int pack(int pkt_no, char *sendpacket)
{
    int i, packsize;
    struct icmp *icmp;
    struct timeval *tval;
    icmp = (struct icmp *)sendpacket;
    icmp->icmp_type = ICMP_ECHO; //设置类型为ICMP请求报文
    icmp->icmp_code = 0;
    icmp->icmp_cksum = 0;
    icmp->icmp_seq = pkt_no;
    icmp->icmp_id = pid;         //设置当前进程ID为ICMP标示符
    packsize = ICMP_HEADSIZE + sizeof(struct timeval);
    tval = (struct timeval *)icmp->icmp_data;
    gettimeofday(tval, NULL);
    icmp->icmp_cksum = cal_chksum((unsigned short *)icmp, packsize);
    return packsize;
}


unsigned short cal_chksum(unsigned short *addr, int len)
{
    int nleft = len;
    int sum = 0;
    unsigned short *w = addr;
    unsigned short answer = 0;
    while (nleft > 1)    //把ICMP报头二进制数据以2字节为单位累加起来
    {
        sum += *w++;
        nleft -= 2;
    }
    if (nleft ==
            1)     //若ICMP报头为奇数个字节,会剩下最后一字节.把最后一个字节视为一个2字节数据的高字节,这个2字节数据的低字节为0,继续累加
    {
        *(unsigned char *)(&answer) = *(unsigned char *)w;
        sum += answer;
    }
    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    answer = ~sum;
    return answer;
}


int recv_packet(int pkt_no, char *recvpacket)
{
    int n, fromlen;
    fd_set rfds;
    FD_ZERO(&rfds);
    FD_SET(sockfd, &rfds);
    signal(SIGALRM, timeout);
    fromlen = sizeof(recv_addr);
    alarm(MAX_WAIT_TIME);
    while (1)
    {
        select(sockfd + 1, &rfds, NULL, NULL, NULL);
        if (FD_ISSET(sockfd, &rfds))
        {
            if ((n = recvfrom(sockfd, recvpacket, PACKET_SIZE, 0, (struct sockaddr *)&recv_addr, &fromlen)) < 0)
            {
                if (errno == EINTR)
                    return -1;
                perror("recvfrom error");
                return -2;
            }
        }
        gettimeofday(&tvrecv, NULL);
        if (unpack(pkt_no, recvpacket, n) == -1)
            continue;
        return 1;
    }
}

int unpack(int cur_seq, char *buf, int len)
{
    int iphdrlen;
    struct ip *ip;
    struct icmp *icmp;
    ip = (struct ip *)buf;
    iphdrlen = ip->ip_hl << 2; //求ip报头长度,即ip报头的长度标志乘4
    icmp = (struct icmp *)(buf + iphdrlen); //越过ip报头,指向ICMP报头
    len -= iphdrlen;    //ICMP报头及ICMP数据报的总长度
    if (len < 8)
        return -1;
    if ((icmp->icmp_type == ICMP_ECHOREPLY) && (icmp->icmp_id == pid) && (icmp->icmp_seq == cur_seq))
        return 0;
    else
        return -1;
}


void timeout(int signo)
{
    printf("Request Timed Out\n");
}

void tv_sub(struct timeval *out, struct timeval *in)
{
    if ((out->tv_usec -= in->tv_usec) < 0)
    {
        --out->tv_sec;
        out->tv_usec += 1000000;
    }
    out->tv_sec -= in->tv_sec;
}

void _CloseSocket()
{
    close(sockfd);
    sockfd = 0;
}



int main(int argc, char *argv[])
{
    printf("network is %d\n", NetIsOk());
    return 0;
}
