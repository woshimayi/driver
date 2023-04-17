/*
 * @*************************************: 
 * @FilePath: /network/ifname_status/get_ifname_status.c
 * @version: 
 * @Author: dof
 * @Date: 2023-02-16 09:30:59
 * @LastEditors: dof
 * @LastEditTime: 2023-02-16 09:33:39
 * @Descripttion:  获取接口状态
 * @**************************************: 
 */

#include <sys/types.h>
#include <sys/socket.h>

#include <sys/ioctl.h>
#include <net/if.h>		/* ifreq struct         */

int get_sock_intf(const char *devnam){
    struct ifreq ifr;
    int retval = 0;
    int isock = 0;

    if ((isock = socket(PF_PACKET, SOCK_DGRAM, 0)) < 0 /* BRCM, Was 1, changed to 0 */)
	return 0;

    strncpy(ifr.ifr_name, devnam, sizeof(ifr.ifr_name));

    retval = ioctl(isock , SIOCGIFFLAGS, &ifr);;
    
    if (isock >= 0) /* BRCM: valid isock is 0 or greater */
	close(isock);

    if (( retval < 0 ) || !(ifr.ifr_flags & IFF_UP))
	return 0;
    else
	return 1;
}


int main(int argc, char const *argv[])
{
	int ret = get_sock_intf("enp0s3");
	printf("status = %d\n", ret);
	return 0;
}
