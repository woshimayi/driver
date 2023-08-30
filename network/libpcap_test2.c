/*
 * @*************************************:
 * @FilePath: /network/libpcap_test2.c
 * @version:
 * @Author: dof
 * @Date: 2023-08-28 10:22:49
 * @LastEditors: dof
 * @LastEditTime: 2023-08-28 10:30:12
 * @Descripttion: libnpcap 加入定时抓包终端程序
 * build:  gcc -o packet_sniffer libpcap_test2.c -lpcap -lrt
 * run:    ./packet_sniffer eth0 capture.pcap
 * @**************************************:
 */

#include <stdio.h>
#include <stdlib.h>
#include <pcap.h>
#include <signal.h>
#include <sys/time.h>

#define SNAP_LEN 1518		  // 以太网帧最大长度
#define SIZE_ETHERNET 14	  // 以太网帧首部长度
#define MAX_FILTER_LENGTH 100 // 过滤器字符串长度的最大值

pcap_t *handle; // 全局变量，用于在信号处理函数中关闭会话

void packet_handler(u_char *user_data, const struct pcap_pkthdr *pkthdr, const u_char *packet);
void signal_handler(int signum);

int main(int argc, char *argv[])
{
	pcap_dumper_t *dumper;
	char errbuf[PCAP_ERRBUF_SIZE];
	struct pcap_pkthdr header;
	const u_char *packet;
	char filter_expression[MAX_FILTER_LENGTH];

	if (argc != 5)
	{
		fprintf(stderr, "Usage: %s <interface> <output_file> <ip_address> <port>\n", argv[0]);
		return 1;
	}

	// 打开网络接口进行抓包
	handle = pcap_open_live(argv[1], SNAP_LEN, 1, 1000, errbuf);
	if (handle == NULL)
	{
		fprintf(stderr, "Could not open device %s: %s\n", argv[1], errbuf);
		return 2;
	}

	// 创建一个用于保存数据包的pcap_dumper_t对象
	dumper = pcap_dump_open(handle, argv[2]);
	if (dumper == NULL)
	{
		fprintf(stderr, "Could not open output file: %s\n", pcap_geterr(handle));
		return 2;
	}

	// 构建过滤器表达式，用于只捕获指定IP地址、端口和协议的数据包
	if (snprintf(filter_expression, MAX_FILTER_LENGTH, "host %s and port %s and %s", argv[3], argv[4], argv[5]) >= MAX_FILTER_LENGTH)
	{
		fprintf(stderr, "Filter expression too long.\n");
		return 3;
	}

	// 编译并应用过滤器
	struct bpf_program filter;
	if (pcap_compile(handle, &filter, filter_expression, 0, PCAP_NETMASK_UNKNOWN) == -1)
	{
		fprintf(stderr, "Could not parse filter %s: %s\n", filter_expression, pcap_geterr(handle));
		return 4;
	}
	if (pcap_setfilter(handle, &filter) == -1)
	{
		fprintf(stderr, "Could not install filter %s: %s\n", filter_expression, pcap_geterr(handle));
		return 5;
	}

	// 注册信号处理程序，在接收到SIGINT信号（如Ctrl+C）时关闭会话
	signal(SIGINT, signal_handler);

	// 设置定时器，10秒后发送 SIGALRM 信号（定时器到期）
	struct itimerval timer;
	timer.it_value.tv_sec = 10;
	timer.it_value.tv_usec = 0;
	timer.it_interval.tv_sec = 0;
	timer.it_interval.tv_usec = 0;
	setitimer(ITIMER_REAL, &timer, NULL);

	// 开始捕获数据包
	pcap_loop(handle, -1, packet_handler, (u_char *)dumper);

	// 关闭并释放dumper对象
	pcap_dump_close(dumper);

	return 0;
}

void packet_handler(u_char *user_data, const struct pcap_pkthdr *pkthdr, const u_char *packet)
{
	pcap_dumper_t *dumper = (pcap_dumper_t *)user_data;

	// 保存数据包到文件
	pcap_dump((u_char *)dumper, pkthdr, packet);
}

void signal_handler(int signum)
{
	// 收到信号后关闭会话
	pcap_close(handle);
	exit(0);
}
