/*
 * @FilePath: /network/get_network_if_device_status.c
 * @version: 
 * @Author: sueRimn
 * @Date: 2020-11-27 16:45:15
 * @LastEditors: sueRimn
 * @LastEditTime: 2021-02-25 17:02:40
 * @Descripttion: 
 */
#if 0
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>        
#include <unistd.h>       
#include <signal.h>        
#include <string.h>        
#include <sys/time.h>    
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/wireless.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <asm/types.h>
#include <linux/netlink.h>
#include <linux/socket.h>
#include <errno.h>
#include <time.h>
#include <signal.h>
#include <sys/file.h> 
#include <syslog.h>


struct nlmsghdr *g_nlh = NULL;
int g_netlink_2g_fd = 0;

int efence_open_netlink_listen()
{
        int ret = -1;
        int sock_fd;
        struct iovec iov;
        struct msghdr msg;
        struct nlmsghdr *nlh = NULL;
        struct sockaddr_nl src_addr, dst_addr;

        sock_fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_STA_MSG);
        if(sock_fd < 0)
        {
                printf("[%s]: call socket fail!\n", __FUNCTION__);
                return -1;
        }

        memset(&src_addr, 0, sizeof(src_addr));
        src_addr.nl_family = AF_NETLINK;
        src_addr.nl_pid    = 0;
        src_addr.nl_groups = 0;

        ret = bind(sock_fd, (struct sockaddr*)&src_addr, sizeof(src_addr));
        if(ret < 0)
        {
                printf("[%s]: call bind fail!\n", __FUNCTION__);
                close(sock_fd);
                return -1;
        }

        nlh = (struct nlmsghdr *)malloc(32);
    if(!nlh)
    {
        printf("[%s]: call malloc fail!\n", __FUNCTION__);
        close(sock_fd);
        return -1;
    }

        memset(&dst_addr, 0, sizeof(dst_addr));
        dst_addr.nl_family = AF_NETLINK;
        dst_addr.nl_pid    = 0;
        dst_addr.nl_groups = 0;

        nlh->nlmsg_len   = NLMSG_SPACE(32);
        nlh->nlmsg_pid   = getpid();
        nlh->nlmsg_flags = 0;

         strcpy(NLMSG_DATA(nlh),"Hello you!");

        iov.iov_base = (void *)nlh;
        iov.iov_len  = NLMSG_SPACE(32);

        memset(&msg, 0, sizeof(msg));
        msg.msg_name    = (void *)&dst_addr;
        msg.msg_namelen = sizeof(dst_addr);
        msg.msg_iov     = &iov;
        msg.msg_iovlen  = 1;


        printf("g_pid = %d\n", nlh->nlmsg_pid);

    ret = sendmsg(sock_fd, &msg, 0);
    if(ret <= 0)
    {
        printf("[%s]: call sendmsg fail!\n", __FUNCTION__);
        close(sock_fd);
        return -1;
    }


    g_netlink_2g_fd = sock_fd;
    return 0;
}



/* ap receive ue macs from netlink */
int efence_receive_fdb_info_from_kernel(int socket_fd)
{

        int ret = -1;
        struct iovec iov;
        struct msghdr msg;
        struct nlmsghdr *nlh = NULL;


#if 0
        nlh = (struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_PAYLOAD));
        if(!nlh)
        {
                printf("[%s]: call malloc fail!\n", __FUNCTION__);
                return -1;
        }
#endif

        nlh = g_nlh;

        memset(nlh, 0, NLMSG_SPACE(MAX_PAYLOAD));
        memset(&iov,0, sizeof(iov));
        iov.iov_base = (void *)nlh;
        iov.iov_len  = NLMSG_SPACE(MAX_PAYLOAD);

        memset(&msg, 0, sizeof(msg));
        msg.msg_iov     = &iov;
        msg.msg_iovlen  = 1;
        printf("start_capture1!\n");
        ret = recvmsg(socket_fd, &msg, 0);
        if(ret < 0)
        {
                printf("[%s]: call recvmsg fail!\n", __FUNCTION__);
                goto exit;
        }
        printf("start_capture2!\n");
	{
		char szcmd[64] = {0};
		sprintf(szcmd, "echo start_capture2 >> /var/sta_info");
		system(szcmd);
	}		
        paras_recv_msg_info(((unsigned char *)NLMSG_DATA(nlh)));
exit:
        printf("[%s]: <==finish!\n", __FUNCTION__);
        return 0;
}


int main() {
    int ret = 0;
    ret = efence_open_netlink_listen();
    if (ret != 0)
    {
        printf("[%s]: call efence_open_netlink_listen fail!\n", __FUNCTION__);
        return -1;
    }
    g_nlh = (struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_PAYLOAD));
    if(!g_nlh)
    {
        printf("[%s]: call malloc fail!\n", __FUNCTION__);
        return -1;
    }
    printf("waiting received!\n");
    // Read message from kernel
    efence_receive_fdb_info_from_kernel(g_netlink_2g_fd);
    close(g_netlink_2g_fd);

    return 0;
}

#endif




#include <sys/types.h>  
#include <sys/socket.h>  
#include <asm/types.h>  
#include <linux/netlink.h>  
#include <linux/rtnetlink.h>  
#include <stdlib.h>  
#include <stdio.h>  
#include <sys/ioctl.h>  
#include <linux/if.h>  
#include <string.h>  
  
#define BUFLEN 20480  
  
int main(int argc, char *argv[])  
{  
    int fd, retval;  
    char buf[BUFLEN] = {0};  
    int len = BUFLEN;  
    struct sockaddr_nl addr;  
    struct nlmsghdr *nh;  
    struct ifinfomsg *ifinfo;  
    struct rtattr *attr;  
  
    fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);  
    setsockopt(fd, SOL_SOCKET, SO_RCVBUF, &len, sizeof(len));  
    memset(&addr, 0, sizeof(addr));  
    addr.nl_family = AF_NETLINK;  
    addr.nl_groups = RTNLGRP_LINK;  
    bind(fd, (struct sockaddr*)&addr, sizeof(addr));  
    while ((retval = read(fd, buf, BUFLEN)) > 0)  
    {  
        for (nh = (struct nlmsghdr *)buf; NLMSG_OK(nh, retval); nh = NLMSG_NEXT(nh, retval))  
        {  
            if (nh->nlmsg_type == NLMSG_DONE)  
                break;  
            else if (nh->nlmsg_type == NLMSG_ERROR)  
                return;  
            else if (nh->nlmsg_type != RTM_NEWLINK)  
                continue;  
            ifinfo = NLMSG_DATA(nh);  
            printf("%u: %s", ifinfo->ifi_index,  
                    (ifinfo->ifi_flags & IFF_LOWER_UP) ? "up" : "down" );  
            attr = (struct rtattr*)(((char*)nh) + NLMSG_SPACE(sizeof(*ifinfo)));  
            len = nh->nlmsg_len - NLMSG_SPACE(sizeof(*ifinfo));  
            for (; RTA_OK(attr, len); attr = RTA_NEXT(attr, len))  
            {  
                if (attr->rta_type == IFLA_IFNAME)  
                {  
                    printf(" %s", (char*)RTA_DATA(attr));  
                    break;  
                }  
            }  
            printf("\n");  
        }  
    }  
  
    return 0;  
}  