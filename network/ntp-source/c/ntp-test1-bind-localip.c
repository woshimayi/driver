/*
 * @*************************************: 
 * @FilePath: /network/ntp-source/c/1.c
 * @version: 
 * @Author: dof
 * @Date: 2021-11-25 13:30:16
 * @LastEditors: dof
 * @LastEditTime: 2021-11-25 13:58:02
 * @Descripttion:  指定 地址 网卡  发包
 * @**************************************: 
 */

/* ntpclient.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <unistd.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <endian.h>
#include <net/if.h>

#define VERSION_3 3
#define VERSION_4 4

#define MODE_CLIENT 3
#define MODE_SERVER 4

#define NTP_LI 0
#define NTP_VN VERSION_3
#define NTP_MODE MODE_CLIENT
#define NTP_STRATUM 0
#define NTP_POLL 4
#define NTP_PRECISION -6

#define NTP_HLEN 48

#define NTP_PORT 123
#define NTP_SERVER "182.92.12.11"

#define TIMEOUT 10

#define BUFSIZE 1500

#define JAN_1970 0x83aa7e80

#define NTP_CONV_FRAC32(x) (uint64_t)((x) * ((uint64_t)1 << 32))
#define NTP_REVE_FRAC32(x) ((double)((double)(x) / ((uint64_t)1 << 32)))

#define NTP_CONV_FRAC16(x) (uint32_t)((x) * ((uint32_t)1 << 16))
#define NTP_REVE_FRAC16(x) ((double)((double)(x) / ((uint32_t)1 << 16)))

#define USEC2FRAC(x) ((uint32_t)NTP_CONV_FRAC32((x) / 1000000.0))
#define FRAC2USEC(x) ((uint32_t)NTP_REVE_FRAC32((x)*1000000.0))

#define NTP_LFIXED2DOUBLE(x) ((double)(ntohl(((struct l_fixedpt *)(x))->intpart) - JAN_1970 + FRAC2USEC(ntohl(((struct l_fixedpt *)(x))->fracpart)) / 1000000.0))

struct s_fixedpt
{
    uint16_t intpart;
    uint16_t fracpart;
};

struct l_fixedpt
{
    uint32_t intpart;
    uint32_t fracpart;
};

struct ntphdr
{
#if __BYTE_ORDER == __BID_ENDIAN
    unsigned int ntp_li : 2;
    unsigned int ntp_vn : 3;
    unsigned int ntp_mode : 3;
#endif
#if __BYTE_ORDER == __LITTLE_ENDIAN
    unsigned int ntp_mode : 3;
    unsigned int ntp_vn : 3;
    unsigned int ntp_li : 2;
#endif
    uint8_t ntp_stratum;
    uint8_t ntp_poll;
    int8_t ntp_precision;
    struct s_fixedpt ntp_rtdelay;
    struct s_fixedpt ntp_rtdispersion;
    uint32_t ntp_refid;
    struct l_fixedpt ntp_refts;
    struct l_fixedpt ntp_orits;
    struct l_fixedpt ntp_recvts;
    struct l_fixedpt ntp_transts;
};

in_addr_t inet_host(const char *host)
{
    in_addr_t saddr;
    struct hostent *hostent;

    if ((saddr = inet_addr(host)) == INADDR_NONE)
    {
        if ((hostent = gethostbyname(host)) == NULL)
            return INADDR_NONE;

        memmove(&saddr, hostent->h_addr, hostent->h_length);
    }

    return saddr;
}

int get_ntp_packet(void *buf, size_t *size)
{
    struct ntphdr *ntp;
    struct timeval tv;

    if (!size || *size < NTP_HLEN)
        return -1;

    memset(buf, 0, *size);

    ntp = (struct ntphdr *)buf;
    ntp->ntp_li = NTP_LI;
    ntp->ntp_vn = NTP_VN;
    ntp->ntp_mode = NTP_MODE;
    ntp->ntp_stratum = NTP_STRATUM;
    ntp->ntp_poll = NTP_POLL;
    ntp->ntp_precision = NTP_PRECISION;

    gettimeofday(&tv, NULL);
    ntp->ntp_transts.intpart = htonl(tv.tv_sec + JAN_1970);
    ntp->ntp_transts.fracpart = htonl(USEC2FRAC(tv.tv_usec));

    *size = NTP_HLEN;

    return 0;
}

void print_ntp(struct ntphdr *ntp)
{
    time_t time;

    printf("LI:\t%d \n", ntp->ntp_li);
    printf("VN:\t%d \n", ntp->ntp_vn);
    printf("Mode:\t%d \n", ntp->ntp_mode);
    printf("Stratum:\t%d \n", ntp->ntp_stratum);
    printf("Poll:\t%d \n", ntp->ntp_poll);
    printf("precision:\t%d \n", ntp->ntp_precision);

    printf("Route delay:\t %lf \n",
           ntohs(ntp->ntp_rtdelay.intpart) + NTP_REVE_FRAC16(ntohs(ntp->ntp_rtdelay.fracpart)));
    printf("Route Dispersion:\t%lf \n",
           ntohs(ntp->ntp_rtdispersion.intpart) + NTP_REVE_FRAC16(ntohs(ntp->ntp_rtdispersion.fracpart)));
    printf("Referencd ID:\t %d \n", ntohl(ntp->ntp_refid));

    time = ntohl(ntp->ntp_refts.intpart) - JAN_1970;
    printf("Reference:\t%d %ld %s \n",
           ntohl(ntp->ntp_refts.intpart) - JAN_1970,
           FRAC2USEC(ntohl(ntp->ntp_refts.fracpart)),
           ctime(&time));

    time = ntohl(ntp->ntp_orits.intpart) - JAN_1970;
    printf("Originate:\t%d %d frac=%ld (%s) \n",
           ntohl(ntp->ntp_orits.intpart) - JAN_1970,
           FRAC2USEC(ntohl(ntp->ntp_orits.fracpart)),
           ntohl(ntp->ntp_orits.fracpart),
           ctime(&time));

    time = ntohl(ntp->ntp_recvts.intpart) - JAN_1970;
    printf("Receive:\t%d %d (%s) \n",
           ntohl(ntp->ntp_recvts.intpart) - JAN_1970,
           FRAC2USEC(ntohl(ntp->ntp_recvts.fracpart)),
           ctime(&time));

    time = ntohl(ntp->ntp_transts.intpart) - JAN_1970;
    printf("Transmit:\t%d %d (%s) \n",
           ntohl(ntp->ntp_transts.intpart) - JAN_1970,
           FRAC2USEC(ntohl(ntp->ntp_transts.fracpart)),
           ctime(&time));
}

double get_rrt(const struct ntphdr *ntp, const struct timeval *recvtv)
{
    double t1, t2, t3, t4;

    t1 = NTP_LFIXED2DOUBLE(&ntp->ntp_orits);
    t2 = NTP_LFIXED2DOUBLE(&ntp->ntp_recvts);
    t3 = NTP_LFIXED2DOUBLE(&ntp->ntp_transts);
    t4 = recvtv->tv_sec + recvtv->tv_usec / 1000000.0;

    return (t4 - t1) - (t3 - t2);
}

double get_offset(const struct ntphdr *ntp, const struct timeval *recvtv)
{
    double t1, t2, t3, t4;

    t1 = NTP_LFIXED2DOUBLE(&ntp->ntp_orits);
    t2 = NTP_LFIXED2DOUBLE(&ntp->ntp_recvts);
    t3 = NTP_LFIXED2DOUBLE(&ntp->ntp_transts);
    t4 = recvtv->tv_sec + recvtv->tv_usec / 1000000.0;

    return ((t2 - t1) + (t3 - t4)) / 2;
}

void usage(void)
{
    fprintf(stderr,
            "Usage : ntpclient"
            " destination"
            "\n");
}

int main(int argc, char *argv[])
{
    char buf[BUFSIZE];
    size_t nbytes;
    int sockfd, maxfd1;
    struct sockaddr_in servaddr;
    fd_set readfds;
    struct timeval timeout, recvtv, tv;
    double offset;
    char ipaddr[128] = {0};

    if (argc != 2)
    {
        usage();
        // exit(-1);
        strncpy(ipaddr, "pool.ntp.org", sizeof(ipaddr));
    }
    else
    {
        strncpy(ipaddr, argv[1], sizeof(ipaddr));
    }
    printf("argv = %s\n", ipaddr);

    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(NTP_PORT);
    servaddr.sin_addr.s_addr = inet_host(ipaddr);

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("socket error");
        exit(-1);
    }

#if 0 // 绑定地址 success
    struct sockaddr_in localaddr;
    localaddr.sin_family = AF_INET;
    localaddr.sin_port = htons(NTP_PORT);
    // localaddr.sin_addr.s_addr = inet_host("127.0.0.1");
    localaddr.sin_addr.s_addr = inet_host("172.16.28.84");
    if (0 > bind(sockfd, (struct sockaddr *)&localaddr, sizeof(struct sockaddr_in)))
    {
        perror("bind local ipaddr failed");
        exit(-1);
    }
#else  // 绑定网卡接口 success
    struct ifreq ifr;
    memset(&ifr, 0x00, sizeof(ifr));
    strncpy(ifr.ifr_name, "enp0s3", sizeof("enp0s3"));
    if (0 > setsockopt(sockfd, SOL_SOCKET, SO_BINDTODEVICE, (void *)&ifr, sizeof(struct ifreq)))
    {
        perror("SO_BINDTODEVICE failed");
        exit(-1);
    }
#endif

    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(struct sockaddr)) != 0)
    {
        perror("connect error");
        exit(-1);
    }

    nbytes = BUFSIZE;
    if (get_ntp_packet(buf, &nbytes) != 0)
    {
        fprintf(stderr, "construct ntp request error \n");
        exit(-1);
    }
    send(sockfd, buf, nbytes, 0);

    FD_ZERO(&readfds);
    FD_SET(sockfd, &readfds);
    maxfd1 = sockfd + 1;

    timeout.tv_sec = TIMEOUT;
    timeout.tv_usec = 0;

    if (select(maxfd1, &readfds, NULL, NULL, &timeout) > 0)
    {
        if (FD_ISSET(sockfd, &readfds))
        {
            if ((nbytes = recv(sockfd, buf, BUFSIZE, 0)) < 0)
            {
                perror("recv error");
                exit(-1);
            }

            gettimeofday(&recvtv, NULL);
            offset = get_offset((struct ntphdr *)buf, &recvtv);

            gettimeofday(&tv, NULL);   // 获取当前时间
            tv.tv_sec += (int)offset;
            tv.tv_usec += offset - (int)offset;

            if (settimeofday(&tv, NULL) != 0) // 设置系统时间
            {
                perror("settimeofday error");
                exit(-1);
            }
            printf("%s \n", ctime((time_t *)&tv.tv_sec)); // 返回时间字符串
        }
    }

    close(sockfd);

    return 0;
}