#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <errno.h>
#include <syslog.h>
#include "ifaddrs.h"
#include <linux/sockios.h>
#include <sys/uio.h>
#include <netinet/ip6.h>
#include <netinet/icmp6.h>

#include <netinet/if_ether.h>
#include <net/if_arp.h>

#define MAX_TRY_TIME 2
// #define _LITTLE_ENDIAN
// #define DEBUG 1
#ifdef DEBUG
#define debug(fmt, x...)                                          \
	do                                                            \
	{                                                             \
		printf("%s(Line %d): " fmt, __FUNCTION__, __LINE__, ##x); \
	} while (0)
#else
#define debug(fmt, x...)
#endif

#define PRINTERROR 1
#ifdef PRINTERROR
#define PError(fmt, x...)                                         \
	do                                                            \
	{                                                             \
		printf("%s(Line %d): " fmt, __FUNCTION__, __LINE__, ##x); \
	} while (0)
#else
#define debug(fmt, x...)
#endif

static const unsigned short DNS_FLAG_QR = 0x8000;
static const unsigned short DNS_FLAG_AA = 0x0400;
static const unsigned short DNS_FLAG_TC = 0x0200;
static const unsigned short DNS_FLAG_RD = 0x0100;
static const unsigned short DNS_FLAG_RA = 0x0080;
static const unsigned short DNS_HEAD_SIZE = 12;

static const unsigned short DNS_RRTYPE_A = 1;
static const unsigned short DNS_RRTYPE_AAAA = 0X1C;
static const unsigned short DNS_RRTYPE_NS = 2;
static const unsigned short DNS_RRTYPE_CNAME = 5;
static const unsigned short DNS_RRTYPE_SOA = 6;
static const unsigned short DNS_RRTYPE_WKS = 11;
static const unsigned short DNS_RRTYPE_PTR = 12;
static const unsigned short DNS_RRTYPE_HINFO = 13;
static const unsigned short DNS_RRTYPE_MX = 15;
static const unsigned short DNS_TAIL_SIZE = 4;

struct DNS_REQUEST_HEAD
{
	unsigned short usSessionID;
	union
	{
		unsigned short allFlag;
#if __BYTE_ORDER == __LITTLE_ENDIAN
		struct
		{
			unsigned short Reply_code : 4;
			unsigned short reserved : 3;
			unsigned short Recursion_available : 1;
			unsigned short Recursion_desired : 1;
			unsigned short Truncated : 1;
			unsigned short Authoritative : 1;
			unsigned short Opcode : 4;
			unsigned short Response : 1;
		} bits;
#else
		struct
		{
			unsigned short Response : 1;
			unsigned short Opcode : 4;
			unsigned short Authoritative : 1;
			unsigned short Truncated : 1;
			unsigned short Recursion_desired : 1;
			unsigned short Recursion_available : 1;
			unsigned short reserved : 3;
			unsigned short Reply_code : 4;
		} bits;
#endif
	} usFlag;
	unsigned short usQuestions;
	unsigned short usAnswer;
	unsigned short usAuthority;
	unsigned short usAdditional;
};

struct DNS_REQUEST_TAIL
{
	unsigned short usRequestType;
	unsigned short usRequestClass;
};

struct SEND_PACKAGE
{
	char *pData;
	int iDataLen;
};

typedef enum
{
	PARAMETERS_IS_NULL = 0,
	TRANS_DNSSERVER_ERROR,
	SOCKET_CREATE_FAIL,
	CLIENT_IP_ERROR,
	BIND_IP_ERROR,
	SEND_DNS_REQUEST_FAIL,
	READ_SOCKET_FAIL,
	SEND_SUCCESS,
	PARSE_DNS_SUCCESS,
	GET_IFNAMEIP_ERROR,
	GET_IFNAMEIP_SUCCESS,
	GET_DNSSERVER_ERROR,
	GET_DNSSERVER_SUCCESS,
	PARSE_DNS_ERROR,
} ParseRet;

ParseRet ParaseIpv4Domain(const char *ifname, const char *strdomain, const char *dnsserver, char *domainIp);
char IsIpv4Addr(char *str);
ParseRet ifnameGetIPv4addr(const char *ifname, char *hoststr);
ParseRet ParaseIpv6Domain(const char *ifname, const char *strdomain, const char *dnsserver, char *domainIp, const char IsIpv6);
char IsIpv6Addr(char *str);