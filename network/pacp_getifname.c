#include <pcap.h>
#include <stdio.h>

int main()
{
    pcap_if_t *alldevs;
    char errbuf[PCAP_ERRBUF_SIZE];

    // 获取所有网络设备
    if (pcap_findalldevs(&alldevs, errbuf) == -1)
    {
        fprintf(stderr, "Error in pcap_findalldevs: %s\n", errbuf);
        exit(1);
    }

    // 遍历所有设备
    for (pcap_if_t *d = alldevs; d; d = d->next)
    {
        printf("%s: ", d->name);
        if (d->addresses)
            printf("%s|%s|%s|%s|\n", d->addresses->addr->sa_data, d->addresses->broadaddr->sa_data, d->addresses->dstaddr->sa_data, d->addresses->netmask->sa_data);
        else
            printf("No description available\n");
    }

    // 释放设备列表
    pcap_freealldevs(alldevs);

    return 0;
}