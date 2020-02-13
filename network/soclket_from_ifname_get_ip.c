/*
* 使用socket 获取网络信息
* ioctl https://baike.baidu.com/item/ioctl/6392403
*
* 网络接口信息
*
* 接口数量:5
* 接口：docker0
* 接口状态: UP
* IP地址:172.17.0.1
* 子网掩码:255.255.0.0
* 广播地址:0.0.0.0
* MAC地址:02:42:3d:e3:4a:04
*
*/

#include <arpa/inet.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>


int fromIfnameGetIp(char **gwip, const char * ifname)
{
    int fd;         /* 套接字 */
    int if_len;     /* 接口数量 */
    struct ifreq buf[16];    /* ifreq结构数组 */
    struct ifconf ifc;                  /* ifconf结构 */

    /* 建立IPv4的UDP套接字fd */
    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    {
        perror("socket(AF_INET, SOCK_DGRAM, 0)");
        return -1;
    }

    /* 初始化ifconf结构 */
    ifc.ifc_len = sizeof(buf);
    ifc.ifc_buf = (caddr_t) buf;

    /* 获得接口列表 */
    if (ioctl(fd, SIOCGIFCONF, (char *) &ifc) == -1)
    {
        perror("SIOCGIFCONF ioctl");
        return -1;
    }

    if_len = ifc.ifc_len / sizeof(struct ifreq); /* 接口数量 */
    printf("接口数量:%d\n\n", if_len);

    /* 遍历每个接口 */
    while (if_len-- != 0)
    {
        if (strstr(buf[if_len].ifr_name, ifname))
        {
            printf("接口：%s\n", buf[if_len].ifr_name); /* 接口名称 */
            /* IP地址 */
            if (!(ioctl(fd, SIOCGIFADDR, (char *) &buf[if_len])))
            {
                printf("IP地址:%s\n",
                    (char *)inet_ntoa(((struct sockaddr_in *)(&buf[if_len].ifr_addr))->sin_addr));
                *gwip = strdup((char *)inet_ntoa(((struct sockaddr_in *)(&buf[if_len].ifr_addr))->sin_addr));
            }
            else
            {
                char str[256];
                sprintf(str, "SIOCGIFADDR ioctl %s\n", buf[if_len].ifr_name);
                perror(str);
            }
        }
    }//–while end

    //关闭socket
    close(fd);
    return 0;
}

int main(int argc, char *argv[])
{
    char *gwip = NULL;
    
    fromIfnameGetIp(&gwip, argv[1]);
    printf("ip = %s\n", gwip);

    return 0;
}