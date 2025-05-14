#include "dns_parse.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <netinet/udp.h>
#include <errno.h>
#include <time.h>

// DNS头部结构
struct dns_header
{
	unsigned short id;
	unsigned char rd : 1;
	unsigned char tc : 1;
	unsigned char aa : 1;
	unsigned char opcode : 4;
	unsigned char qr : 1;
	unsigned char rcode : 4;
	unsigned char cd : 1;
	unsigned char ad : 1;
	unsigned char z : 1;
	unsigned char ra : 1;
	unsigned short qdcount;
	unsigned short ancount;
	unsigned short nscount;
	unsigned short arcount;
};

// 创建DNS查询报文
static int create_dns_query(const char *domain, unsigned char *buffer, int buffer_size, bool ipv6_query)
{
	if (domain == NULL || buffer == NULL || buffer_size < 512)
	{
		return -1;
	}

	struct dns_header *header = (struct dns_header *)buffer;
	header->id = htons(0x1234);
	header->qr = 0;
	header->opcode = 0;
	header->aa = 0;
	header->tc = 0;
	header->rd = 1;
	header->ra = 0;
	header->z = 0;
	header->ad = 0;
	header->cd = 0;
	header->rcode = 0;
	header->qdcount = htons(1);
	header->ancount = 0;
	header->nscount = 0;
	header->arcount = 0;

	unsigned char *query = buffer + sizeof(struct dns_header);
	const char *dot;
	const char *label = (char *)domain;
	int pos = 0;

	while ((dot = strchr(label, '.')) != NULL)
	{
		int len = dot - label;
		query[pos++] = len;
		memcpy(query + pos, label, len);
		pos += len;
		label = dot + 1;
	}

	int len = strlen(label);
	query[pos++] = len;
	memcpy(query + pos, label, len);
	pos += len;

	query[pos++] = 0;
	if (ipv6_query)
	{
		query[pos++] = 0;
		query[pos++] = 28; // AAAA
	}
	else
	{
		query[pos++] = 0;
		query[pos++] = 1; // A
	}
	query[pos++] = 0;
	query[pos++] = 1; // IN

	return sizeof(struct dns_header) + pos;
}

// 绑定到网络接口
static int bind_to_interface(int sockfd, const char *ifname)
{
	if (ifname == NULL)
	{
		return 0;
	}

	struct ifreq ifr;
	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, ifname, IFNAMSIZ - 1);

	if (setsockopt(sockfd, SOL_SOCKET, SO_BINDTODEVICE, (void *)&ifr, sizeof(ifr)))
	{
		return -1;
	}

	return 0;
}

// 解析DNS响应
static void parse_dns_response(unsigned char *buffer, int len, struct dns_result *result)
{
	if (len < sizeof(struct dns_header))
	{
		snprintf(result->error_message, sizeof(result->error_message), "Invalid DNS response length");
		result->error_code = -1;
		return;
	}

	struct dns_header *header = (struct dns_header *)buffer;
	if (header->rcode != 0)
	{
		snprintf(result->error_message, sizeof(result->error_message), "DNS error code: %d", header->rcode);
		result->error_code = -2;
		return;
	}

	unsigned char *ptr = buffer + sizeof(struct dns_header);
	int i;
	for (i = 0; i < ntohs(header->qdcount); i++)
	{
		while (*ptr != 0)
		{
			if ((*ptr & 0xC0) == 0xC0)
			{
				ptr += 2;
				break;
			}
			ptr += *ptr + 1;
		}
		ptr += 5;
	}

	for (i = 0; i < ntohs(header->ancount); i++)
	{
		if ((ptr[0] & 0xC0) == 0xC0)
		{
			ptr += 2;
		}
		else
		{
			while (*ptr != 0)
			{
				ptr += *ptr + 1;
			}
			ptr += 1;
		}

		unsigned short type = ntohs(*(unsigned short *)ptr);
		ptr += 2;
		ptr += 2;
		ptr += 4;
		unsigned short rdlength = ntohs(*(unsigned short *)ptr);
		ptr += 2;

		if (type == 1 && rdlength == 4)
		{
			struct in_addr addr;
			memcpy(&addr, ptr, 4);
			inet_ntop(AF_INET, &addr, result->ipv4_address, sizeof(result->ipv4_address));
		}
		else if (type == 28 && rdlength == 16)
		{
			struct in6_addr addr6;
			memcpy(&addr6, ptr, 16);
			inet_ntop(AF_INET6, &addr6, result->ipv6_address, sizeof(result->ipv6_address));
		}
		else if (type == 5)
		{
			unsigned char *name_ptr = ptr;
			int cname_pos = 0;
			while (*name_ptr != 0 && cname_pos < sizeof(result->cname) - 1)
			{
				if ((*name_ptr & 0xC0) == 0xC0)
				{
					unsigned short offset = ntohs(*(unsigned short *)name_ptr) & 0x3FFF;
					name_ptr = buffer + offset;
				}
				else
				{
					int len = *name_ptr;
					name_ptr++;
					if (cname_pos + len < sizeof(result->cname) - 1)
					{
						memcpy(result->cname + cname_pos, name_ptr, len);
						cname_pos += len;
						name_ptr += len;
						if (*name_ptr != 0)
						{
							result->cname[cname_pos++] = '.';
						}
					}
				}
			}
			result->cname[cname_pos] = '\0';
		}
		ptr += rdlength;
	}
}

struct dns_result dns_query(const char *domain,
							const char *dns_server,
							int query_type,
							const char *interface,
							int timeout_sec)
{
	struct dns_result result = {0};

	if (domain == NULL || dns_server == NULL || (query_type != 4 && query_type != 6))
	{
		snprintf(result.error_message, sizeof(result.error_message), "Invalid parameters");
		result.error_code = -1;
		return result;
	}

	bool ipv6_query = (query_type == 6);
	bool ipv6_server = (strchr(dns_server, ':') != NULL);

	int sockfd = socket(ipv6_server ? AF_INET6 : AF_INET, SOCK_DGRAM, 0);
	if (sockfd < 0)
	{
		snprintf(result.error_message, sizeof(result.error_message), "Socket creation failed: %s", strerror(errno));
		result.error_code = -2;
		return result;
	}

	if (bind_to_interface(sockfd, interface) != 0)
	{
		snprintf(result.error_message, sizeof(result.error_message), "Failed to bind to interface: %s", strerror(errno));
		close(sockfd);
		result.error_code = -3;
		return result;
	}

	struct timeval timeout = {timeout_sec, 0};
	if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)))
	{
		snprintf(result.error_message, sizeof(result.error_message), "Failed to set timeout: %s", strerror(errno));
		close(sockfd);
		result.error_code = -4;
		return result;
	}

	struct sockaddr_storage servaddr;
	memset(&servaddr, 0, sizeof(servaddr));
	socklen_t servaddr_len;

	if (ipv6_server)
	{
		struct sockaddr_in6 *servaddr6 = (struct sockaddr_in6 *)&servaddr;
		servaddr6->sin6_family = AF_INET6;
		servaddr6->sin6_port = htons(53);
		if (inet_pton(AF_INET6, dns_server, &servaddr6->sin6_addr) <= 0)
		{
			snprintf(result.error_message, sizeof(result.error_message), "Invalid IPv6 DNS server address");
			close(sockfd);
			result.error_code = -5;
			return result;
		}
		servaddr_len = sizeof(struct sockaddr_in6);
	}
	else
	{
		struct sockaddr_in *servaddr4 = (struct sockaddr_in *)&servaddr;
		servaddr4->sin_family = AF_INET;
		servaddr4->sin_port = htons(53);
		if (inet_pton(AF_INET, dns_server, &servaddr4->sin_addr) <= 0)
		{
			snprintf(result.error_message, sizeof(result.error_message), "Invalid IPv4 DNS server address");
			close(sockfd);
			result.error_code = -6;
			return result;
		}
		servaddr_len = sizeof(struct sockaddr_in);
	}

	unsigned char sendbuf[512], recvbuf[512];
	int query_len = create_dns_query(domain, sendbuf, sizeof(sendbuf), ipv6_query);
	if (query_len < 0)
	{
		snprintf(result.error_message, sizeof(result.error_message), "Failed to create DNS query");
		close(sockfd);
		result.error_code = -7;
		return result;
	}

	ssize_t sent = sendto(sockfd, sendbuf, query_len, 0,
						  (const struct sockaddr *)&servaddr, servaddr_len);
	if (sent != query_len)
	{
		snprintf(result.error_message, sizeof(result.error_message), "Failed to send query: %s", strerror(errno));
		close(sockfd);
		result.error_code = -8;
		return result;
	}

	struct sockaddr_storage from_addr;
	socklen_t from_len = sizeof(from_addr);
	ssize_t recv_len = recvfrom(sockfd, recvbuf, sizeof(recvbuf), 0,
								(struct sockaddr *)&from_addr, &from_len);
	if (recv_len < 0)
	{
		if (errno == EAGAIN || errno == EWOULDBLOCK)
		{
			snprintf(result.error_message, sizeof(result.error_message), "DNS query timed out");
			result.error_code = -9;
		}
		else
		{
			snprintf(result.error_message, sizeof(result.error_message), "Failed to receive response: %s", strerror(errno));
			result.error_code = -10;
		}
		close(sockfd);
		return result;
	}

	parse_dns_response(recvbuf, recv_len, &result);
	close(sockfd);
	return result;
}