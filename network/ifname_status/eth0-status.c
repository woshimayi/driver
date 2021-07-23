/*
 * @FilePath: /network/eth0-status.c
 * @version: 
 * @Author: sueRimn
 * @Date: 2020-09-30 16:51:19
 * @LastEditors: sueRimn
 * @LastEditTime: 2021-02-25 17:02:50
 * @Descripttion: 
 */
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <errno.h>
#include <net/if.h>

struct ethtool_value
{
    __uint32_t cmd;
    __uint32_t data;
};

#define SOCK_1 0
#define SOCK_2 1

#if SOCK_1
/*return 1:has cable; return 0:no cable*/
/**
 * [detect_eth_cable description]
 * @param  ifname [description]
 * @return        [1:has cable; return 0:no cable]
 */
int detect_eth_cable(char *ifname)
{
    struct ethtool_value edata;
    struct ifreq ifr;
    int fd = -1, err = 0;
    memset(&ifr, 0, sizeof(ifr));
    strcpy(ifr.ifr_name, ifname);
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0)
    {
        //perror("Cannot get control socket");
        return -1;
    }
    edata.cmd = 0x0000000A;
    ifr.ifr_data = (caddr_t)&edata;
    err = ioctl(fd, 0x8946, &ifr);
    if (err == 0)
    {
        fprintf(stdout, "Link detected: %s\n", edata.data ? "yes" : "no");
    }
    else if (errno != EOPNOTSUPP)
    {
        perror("Cannot get link status");
    }
    return (edata.data == 1 ? 1 : 0);
}

/**
 * [main description]
 * @param  argc [description]
 * @param  argv [description]
 * @return      [description]
 */
int main(int argc, char **argv)
{
    detect_eth_cable("lo");
    return 0;
}
#endif

#if SOCK_2
/**
 * [main description]
 * @param  argc [description]
 * @param  argv [description]
 * @return      [description]
 */
int main(int argc, char **argv)
{
    struct ethtool_value edata;
    int fd = -1, err = 0;
    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    strcpy(ifr.ifr_name, "enp1s0");
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0)
    {
        perror("Cannot get control socket");
        return 70;
    }
    edata.cmd = 0x0000000a;
    ifr.ifr_data = (caddr_t)&edata;
    err = ioctl(fd, 0x8946, &ifr);
    if (err == 0)
    {
        fprintf(stdout, "Link detected: %s\n",
                edata.data ? "yes" : "no");
    }
    else if (errno != EOPNOTSUPP)
    {
        perror("Cannot get link status");
    }
    return 0;
}
#endif


