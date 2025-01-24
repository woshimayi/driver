#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <errno.h>
#include <sys/socket.h>
 
/* Kernel Netlink */
int CUSBListener_initSock()
{
    const int buffersize = UEVENT_BUFFER_SIZE;
    int ret;
    int on = 1;
 
    struct sockaddr_nl snl;
    bzero(&snl, sizeof(struct sockaddr_nl));
    snl.nl_family = AF_NETLINK;
    snl.nl_pid = getpid();
    //snl.nl_groups = 1|RTNLGRP_LINK;
    //snl.nl_groups = 1;
    snl.nl_groups = RTMGRP_LINK | RTMGRP_IPV4_IFADDR | RTMGRP_IPV6_IFADDR;
 
    int s = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_KOBJECT_UEVENT);
    if (s == -1)
    {
		return -1;
    }
    if (setsockopt(s, SOL_SOCKET, /*SO_RCVBUFFORCE*/ SO_RCVBUF, &buffersize, sizeof(buffersize)) < 0)
    {
		perror("setsockopet error\n");
		return -1;
    }
    if((setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)))<0)
    {
		perror("setsockopet error\n");
		return -1;
    }
 
    ret = bind(s, (struct sockaddr *)&snl, sizeof(struct sockaddr_nl));
    if (ret < 0)
    {
		return -2;
    }
    return s;
}
 
void CUSBListener_onUSB(const char *msg)
{
    if (!memcmp(msg, "add@", 4))
    {
		// 识别 U盘
		if (!memcmp(&msg[strlen(msg)-5], "/sd", 3)) {
			printf("Found U Disk\n");
			printf("%s\n",&msg[strlen(msg) - 5]);
		} else
		// 识别 GPIB
		if (!memcmp(&msg[strlen(msg)-9], "/usbgpib", 8)) {
			printf("USB_GPIB connected\n");
			system("mknod /dev/usbgpib0 c 180 176"); // 手动创建设备节点
		}
    } else
    if (!memcmp(msg,"remove@",7))
    {
		if (!memcmp(&msg[strlen(msg) - 5],"/sd",3)) {
			printf("remove U Disk\n");
			printf("%s\n",&msg[strlen(msg) - 5]);
		} else
		if (!memcmp(&msg[strlen(msg)-9], "/usbgpib", 8)) {
			printf("USB_GPIB disconnected\n");
			system("rm /dev/usbgpib0"); // 删除设备节点
		}
    }
}
 
/* listener for USB Event message*/
void CUSBListener_run()
{
    char buf[UEVENT_BUFFER_SIZE * 2] = {0};
 
    int sock = CUSBListener_initSock();
 
    while(sock > 0)
    {
		/* Netlink message buffer */
		int b = recv(sock, &buf, sizeof(buf),0);
		if(b > 0)
		{
			//printf("%s\n", buf);
			CUSBListener_onUSB(buf);
		}
    }
    perror("Create Netlink failed:" );
}