#include <stdio.h>
#include <string.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

int hi_get_ifindex_by_devname(char *devname, int *ifindex)
{
    int i_ret = 0;
    int i_sockfd = 0;
    struct ifreq st_ifr;

    i_sockfd = socket(PF_PACKET, SOCK_RAW, 0);
    if (0 > i_sockfd)
    {
        return -1;
    }

    memset(&st_ifr, '\0', sizeof(struct ifreq));
    strncpy(st_ifr.ifr_name, devname, sizeof(st_ifr.ifr_name));
    i_ret = ioctl(i_sockfd, SIOCGIFINDEX, &st_ifr);
    if (0 > i_ret)
    {
        close(i_sockfd);
        return -1;
    }
    *ifindex = st_ifr.ifr_ifindex;
    close(i_sockfd);
    return 0;
}

int main(int argc, char const *argv[])
{
    int index = 0;
    hi_get_ifindex_by_devname("eth0", &index);
    return 0;
}
