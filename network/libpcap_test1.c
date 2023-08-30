/*
 * @*************************************: 
 * @FilePath: /network/libpcap_test1.c
 * @version: 
 * @Author: dof
 * @Date: 2023-08-28 10:16:31
 * @LastEditors: dof
 * @LastEditTime: 2023-08-28 10:16:31
 * @Descripttion: 使用libpcap 抓包保存为文件
 * @**************************************: 
 */


#include <stdio.h>
#include <stdlib.h>
#include <pcap.h>

#define SNAP_LEN 1518   // 以太网帧最大长度
#define SIZE_ETHERNET 14 // 以太网帧首部长度

void packet_handler(u_char *user_data, const struct pcap_pkthdr *pkthdr, const u_char *packet);

int main(int argc, char *argv[]) {
    pcap_t *handle;
    pcap_dumper_t *dumper;
    char errbuf[PCAP_ERRBUF_SIZE];
    struct pcap_pkthdr header;
    const u_char *packet;

    // 打开网络接口进行抓包
    handle = pcap_open_live(argv[1], SNAP_LEN, 1, 1000, errbuf);
    if (handle == NULL) {
        fprintf(stderr, "Could not open device %s: %s\n", argv[1], errbuf);
        return 2;
    }

    // 创建一个用于保存数据包的pcap_dumper_t对象
    dumper = pcap_dump_open(handle, argv[2]);
    if (dumper == NULL) {
        fprintf(stderr, "Could not open output file: %s\n", pcap_geterr(handle));
        return 2;
    }

    // 开始捕获数据包
    pcap_loop(handle, 0, packet_handler, (u_char *)dumper);

    // 关闭会话
    pcap_close(handle);

    // 关闭并释放dumper对象
    pcap_dump_close(dumper);

    return 0;
}

void packet_handler(u_char *user_data, const struct pcap_pkthdr *pkthdr, const u_char *packet) {
    pcap_dumper_t *dumper = (pcap_dumper_t *)user_data;

    // 保存数据包到文件
    pcap_dump((u_char *)dumper, pkthdr, packet);
}
