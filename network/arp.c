#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <net/if_arp.h>
#include <string.h>



/*******************************************************************************
* 函 数 名 :   arpDel
* 负 责 人 :
* 创建日期  :
* 函数功能  :   LINUX删除ARP表项
* 输入参数  :
                   char *ifname     :   网络接口名，如eth0
                   char *ipStr      :   需要删除的IP，点分十进制串
* 输出参数  :
* 返 回 值 :
                0       :   成功
                -1      :   失败
* 调用关系  :
* 其 它   :
* 修改记录  :
*******************************************************************************/
int arpDel(char *ifname, char *ipStr)
{
    if(ifname == NULL || ipStr == NULL)
    {
        printf("para is null.\n");
        return -1;
    }

    struct arpreq req;
    struct sockaddr_in *sin;
    int ret = 0;
    int sock_fd = 0;

    memset(&req, 0, sizeof(struct arpreq));
    sin = (struct sockaddr_in *)&req.arp_pa;
    sin->sin_family = AF_INET;
    sin->sin_addr.s_addr = inet_addr(ipStr);
    //arp_dev长度为[16]，注意越界
    strncpy(req.arp_dev, ifname, 15);

    sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(sock_fd < 0)
    {
        printf("get socket error.\n");
        return -1;
    }

    ret = ioctl(sock_fd, SIOCDARP, &req);
    if(ret < 0)
    {
        printf("ioctl error.\n");
        close(sock_fd);
        return -1;
    }

    close(sock_fd);
    return 0;
}




/*******************************************************************************
* 函 数 名 :   arpGet
* 负 责 人 :
* 创建日期  :
* 函数功能  :   LINUX获取ARP表项
* 输入参数  :
                   char *ifname     :   网络接口名，如eth0
                   char *ipStr      :   需要删除的IP，点分十进制串
* 输出参数  :
* 返 回 值 :
                0       :   成功
                -1      :   失败
* 调用关系  :
* 其 它   :
* 修改记录  :
*******************************************************************************/
int arpGet(char *ifname, char *ipStr)
{
    if(ifname == NULL || ipStr == NULL)
    {
        printf("para is null.\n");
        return -1;
    }

    struct arpreq req;
    struct sockaddr_in *sin;
    int ret = 0;
    int sock_fd = 0;

    memset(&req, 0, sizeof(struct arpreq));

    sin = (struct sockaddr_in *)&req.arp_pa;
    sin->sin_family = AF_INET;
    sin->sin_addr.s_addr = inet_addr(ipStr);

    //arp_dev长度为[16]，注意越界
    strncpy(req.arp_dev, ifname, 15);

    sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(sock_fd < 0)
    {
        printf("get socket error.\n");
        return -1;
    }

    ret = ioctl(sock_fd, SIOCGARP, &req);
    if(ret < 0)
    {
        printf("ioctl error.\n");
        close(sock_fd);
        return -1;
    }

    unsigned char *hw = (unsigned char *)req.arp_ha.sa_data;
    printf("%#x-%#x-%#x-%#x-%#x-%#x\n", hw[0], hw[1], hw[2], hw[3], hw[4], hw[5]);
    printf("%#x\n", req.arp_flags);
    close(sock_fd);
    return 0;
}





int getHwAddr(char *buff, char *mac)
{
    if( buff == NULL || mac == NULL )
    {
        return -1;
    }

    int i = 0;
    unsigned int p[6];

    if(sscanf(mac, "%x:%x:%x:%x:%x:%x", &p[0], &p[1], &p[2], &p[3], &p[4], &p[5]) < 6)
    {
        return -1;
    }

    for(i = 0; i < 6; i ++)
    {
        buff[i] = p[i];
    }

    return 0;
}



/*******************************************************************************
* 函 数 名 :   arpSet
* 负 责 人 :
* 创建日期  :
* 函数功能  :   LINUX增加ARP表项
* 输入参数  :
                   char *ifname     :   网络接口名，如eth0
                   char *ipStr      :   IP，点分十进制串
                   char *mac        :   MAC地址，如00:a1:b2:c3:d4:e5
* 输出参数  :
* 返 回 值 :
                0       :   成功
                -1      :   失败
* 调用关系  :
* 其 它   :
* 修改记录  :
*******************************************************************************/
int arpSet(char *ifname, char *ipStr, char *mac)
{
    if(ifname == NULL || ipStr == NULL || mac == NULL)
    {
        printf("para is null.\n");
        return -1;
    }

    struct arpreq req;
    struct sockaddr_in *sin;
    int ret = 0;
    int sock_fd = 0;

    memset(&req, 0, sizeof(struct arpreq));
    sin = (struct sockaddr_in *)&req.arp_pa;
    sin->sin_family = AF_INET;
    sin->sin_addr.s_addr = inet_addr(ipStr);
    //arp_dev长度为[16]，注意越界
    strncpy(req.arp_dev, ifname, 15);
    req.arp_flags = ATF_PERM | ATF_COM;

    if(getHwAddr((char *)req.arp_ha.sa_data, mac) < 0)
    {
        printf("get mac error.\n");
        return -1;
    }

    sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(sock_fd < 0)
    {
        printf("get socket error.\n");
        return -1;
    }

    ret = ioctl(sock_fd, SIOCSARP, &req);
    if(ret < 0)
    {
        printf("ioctl error.\n");
        close(sock_fd);
        return -1;
    }

    close(sock_fd);
    return 0;
}



int main(int argc, char *argv[])
{
    printf("---------------------------------------\n");
    arpSet("eth0", "5.5.5.5", "00:a3:b4:c5:d6:e7");
    printf("---------------------------------------\n");
    arpGet("eth0", "5.5.5.5");
    printf("---------------------------------------\n");
    printf("retvalue=%d\n", arpDel("eth0", "5.5.5.5"));
    printf("---------------------------------------\n");
    arpGet("eth0", "5.5.5.5");

    return 0;
}