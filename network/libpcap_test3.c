/*
 * @*************************************:
 * @FilePath: /network/libpcap_test3.c
 * @version:
 * @Author: dof
 * @Date: 2023-08-28 11:33:28
 * @LastEditors: dof
 * @LastEditTime: 2023-08-29 19:35:33
 * @Descripttion: libpcap 抓包定时 参数化
 * @**************************************:
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
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

	int option;
	char *interface = NULL;
	char *output_file = NULL;
	char *protocol = NULL;
	long int timeout = 10;
	char *port = 0;
	char *host = NULL;

	while ((option = getopt(argc, argv, "i:p:w:P:t:")) != -1)
	{
		switch (option)
		{
		case 'i':
			interface = optarg;
			break;
		case 'p':
			port = optarg;
			if (atoi(optarg) < 0 && atoi(optarg) > 65535)
			{
				return -1;
			}
			snprintf(filter_expression, MAX_FILTER_LENGTH, "and %s", port);
			break;
		case 'w':
			output_file = optarg;
			break;
		case 'P':
			protocol = optarg;
			snprintf(filter_expression, MAX_FILTER_LENGTH, "%s and %s", filter_expression, protocol);
			break;
		case 't':
			timeout = atoi(optarg);
			break;
		case '?':
			if (optopt == 'i' || optopt == 'o')
			{
				fprintf(stderr, "Option -%c requires an argument.\n", optopt);
			}
			else
			{
				fprintf(stderr, "Unknown option -%c.\n", optopt);
			}
			return 1;
		default:
			abort();
		}
	}

	host = argv[optind];

	if (snprintf(filter_expression, MAX_FILTER_LENGTH, "%s %s %s %s %s %s", host ? "host " : "", host ? host : "", port ? "and port" : "", port ? port : "", protocol ? "and" : "", protocol ? protocol : "") >= MAX_FILTER_LENGTH)
	{
		fprintf(stderr, "Filter expression too long.\n");
		return 3;
	}

	printf("Input file: %s:%s:%s:%s\n", interface, port, output_file, host);
	printf("%s\n", filter_expression);

	for (int i = 0; i < argc; i++)
	{
		printf("%s ", argv[i]);
	}
	printf("\n");

	// 打开网络接口进行抓包
	handle = pcap_open_live(interface, SNAP_LEN, 1, 1000, errbuf);
	if (handle == NULL)
	{
		fprintf(stderr, "Could not open device %s: %s\n", interface, errbuf);
		return 2;
	}

	// 创建一个用于保存数据包的pcap_dumper_t对象
	dumper = pcap_dump_open(handle, output_file);
	if (dumper == NULL)
	{
		fprintf(stderr, "Could not open output file: %s\n", pcap_geterr(handle));
		return 2;
	}

	// 构建过滤器表达式，用于只捕获指定IP地址、端口和协议的数据包
	// if (snprintf(filter_expression, MAX_FILTER_LENGTH, "host %s and port %d and %s", host, port, protocol) >= MAX_FILTER_LENGTH)
	// {
	// 	fprintf(stderr, "Filter expression too long.\n");
	// 	return 3;
	// }

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
	signal(SIGALRM, signal_handler);

	// 设置定时器，10秒后发送 SIGALRM 信号（定时器到期）
	struct itimerval timer;
	timer.it_value.tv_sec = timeout;
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
