#include "parse_dns.h"

#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/if_ether.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netinet/ip_icmp.h>
#include <time.h>
#include <strings.h>
#include <sys/ioctl.h>

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

#define PP(fmt, args...) printf("\033[0;32;31m[mdm :%s(%d)] " fmt "\033[1;37m\r\n", __func__, __LINE__, ##args)

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

// static const unsigned short DNS_FLAG_QR = 0x8000;
// static const unsigned short DNS_FLAG_AA = 0x0400;
// static const unsigned short DNS_FLAG_TC = 0x0200;
// static const unsigned short DNS_FLAG_RD = 0x0100;
// static const unsigned short DNS_FLAG_RA = 0x0080;
// static const unsigned short DNS_HEAD_SIZE = 12;

// static const unsigned short DNS_RRTYPE_A = 1;
// static const unsigned short DNS_RRTYPE_AAAA = 0X1C;
// static const unsigned short DNS_RRTYPE_NS = 2;
// static const unsigned short DNS_RRTYPE_CNAME = 5;
// static const unsigned short DNS_RRTYPE_SOA = 6;
// static const unsigned short DNS_RRTYPE_WKS = 11;
// static const unsigned short DNS_RRTYPE_PTR = 12;
// static const unsigned short DNS_RRTYPE_HINFO = 13;
// static const unsigned short DNS_RRTYPE_MX = 15;
// static const unsigned short DNS_TAIL_SIZE = 4;

// struct DNS_REQUEST_HEAD
// {
// 	unsigned short usSessionID;
// 	union
// 	{
// 		unsigned short allFlag;
// #if __BYTE_ORDER == __LITTLE_ENDIAN
// 		struct
// 		{
// 			unsigned short Reply_code : 4;
// 			unsigned short reserved : 3;
// 			unsigned short Recursion_available : 1;
// 			unsigned short Recursion_desired : 1;
// 			unsigned short Truncated : 1;
// 			unsigned short Authoritative : 1;
// 			unsigned short Opcode : 4;
// 			unsigned short Response : 1;
// 		} bits;
// #else
// 		struct
// 		{
// 			unsigned short Response : 1;
// 			unsigned short Opcode : 4;
// 			unsigned short Authoritative : 1;
// 			unsigned short Truncated : 1;
// 			unsigned short Recursion_desired : 1;
// 			unsigned short Recursion_available : 1;
// 			unsigned short reserved : 3;
// 			unsigned short Reply_code : 4;
// 		} bits;
// #endif
// 	} usFlag;
// 	unsigned short usQuestions;
// 	unsigned short usAnswer;
// 	unsigned short usAuthority;
// 	unsigned short usAdditional;
// };

// struct DNS_REQUEST_TAIL
// {
// 	unsigned short usRequestType;
// 	unsigned short usRequestClass;
// };

// struct SEND_PACKAGE
// {
// 	char *pData;
// 	int iDataLen;
// };

// typedef enum
// {
// 	PARAMETERS_IS_NULL = 0,
// 	TRANS_DNSSERVER_ERROR,
// 	SOCKET_CREATE_FAIL,
// 	CLIENT_IP_ERROR,
// 	BIND_IP_ERROR,
// 	SEND_DNS_REQUEST_FAIL,
// 	READ_SOCKET_FAIL,
// 	SEND_SUCCESS,
// 	PARSE_DNS_SUCCESS,
// 	GET_IFNAMEIP_ERROR,
// 	GET_IFNAMEIP_SUCCESS,
// 	GET_DNSSERVER_ERROR,
// 	GET_DNSSERVER_SUCCESS,
// 	PARSE_DNS_ERROR,
// } ParseRet;

// ParseRet ParaseIpv4Domain(const char *ifname, const char *strdomain, const char *dnsserver, char *domainIp);
// char IsIpv4Addr(char *str);
// ParseRet ifnameGetIPv4addr(const char *ifname, char *hoststr);
// ParseRet ParaseIpv6Domain(const char *ifname, const char *strdomain, const char *dnsserver, char *domainIp, const char IsIpv6);
// char IsIpv6Addr(char *str);

static char arrayStr[10][50] = {{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}};

static int arraystr_length = 0;
static unsigned short SendSessionID;
static struct SEND_PACKAGE sendPkg;
static struct sockaddr_in6 fromv6;

#define isspace0(c) ((c) == ' ')
#define DMP_DNS_REQUEST 0
#define DMP_DNS_RESPONSE 1

#define DMP_ERROR_ZERO 0
#define DMP_ERROR_FORMAT 1
#define DMP_ERROR_FAIL 2
#define DMP_ERROR_NAME 3
#define DMP_ERROR_NOT_SURPPORT 4
#define DMP_ERROR_REFUSED 5

#define NIP6(addr)                  \
	ntohs((addr).s6_addr16[0]),     \
		ntohs((addr).s6_addr16[1]), \
		ntohs((addr).s6_addr16[2]), \
		ntohs((addr).s6_addr16[3]), \
		ntohs((addr).s6_addr16[4]), \
		ntohs((addr).s6_addr16[5]), \
		ntohs((addr).s6_addr16[6]), \
		ntohs((addr).s6_addr16[7])
#define NIP6_FMT "%04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x"

static void getArrayByDomainStr(const char *Domain)
{
	char byChar = 0;
	char strTemp[50] = {0};
	int fir = 0, w;
	int sec = 0;

	int domainLen;
	domainLen = strlen(Domain);

	PP("getArrayByDomainStr begin");

	// for(w = 0; w < 50; w++)
	for (w = 0; w < domainLen; w++)
	{
		byChar = Domain[w];
		if ('.' == byChar)
		{
			// int t = strlen(strTemp);

			// memcpy(arrayStr[sec], strTemp, t);
			strcpy(arrayStr[sec], strTemp);

			sec++;
			memset(strTemp, 0, 50);
			fir = 0;
		}
		else
		{
			strTemp[fir] = byChar;
			fir++;
		}
	}
	if (strTemp[0] != 0)
	{
		// memcpy(arrayStr[sec], strTemp, strlen(strTemp));
		strcpy(arrayStr[sec], strTemp);

		sec++;
	}
	arraystr_length = sec;
}

static void getPackageRequestIP(struct SEND_PACKAGE *sendPkg, const char *Domain, const char IsIpv6)
{
	char byBuf[526] = {0};
	char byTemp = 0;
	int pDataPtr = 0;
	int iLastLen = 0; // 最终查询报文长度
	int i = 0;
	char strElement[50] = {0};
	struct DNS_REQUEST_HEAD dnsHead;
	struct DNS_REQUEST_TAIL dnsTail;

	PP("getPackageRequestIP begin");
	// head(12字节)   初始化报头字段
	dnsHead.usSessionID = htons(0X0300);
	SendSessionID = dnsHead.usSessionID;
	dnsHead.usFlag.allFlag = htons(DNS_FLAG_RD);
	dnsHead.usQuestions = htons(0X0001);
	dnsHead.usAnswer = htons(0X0000);
	dnsHead.usAuthority = htons(0X0000);
	dnsHead.usAdditional = htons(0X0000);
	memcpy(byBuf + pDataPtr, &dnsHead, DNS_HEAD_SIZE);
	pDataPtr = pDataPtr + DNS_HEAD_SIZE;
	// 构造报文身体
	getArrayByDomainStr(Domain);
	for (i = 0; i < arraystr_length; i++)
	{
		// memcpy(strElement, arrayStr[i], strlen(arrayStr[i]));
		strcpy(strElement, arrayStr[i]);

		// 写长度
		byTemp = strlen(strElement);
		byBuf[pDataPtr] = byTemp;
		++pDataPtr;
		// 写内容
		memcpy(byBuf + pDataPtr, strElement, byTemp);
		memset(strElement, 0, 50);
		pDataPtr = pDataPtr + byTemp;
	}
	// 标识DomainStr已经完了
	byBuf[pDataPtr] = 0X00;
	++pDataPtr;

	// 根据域名查找IP
	dnsTail.usRequestType = IsIpv6 ? htons(DNS_RRTYPE_AAAA) : htons(DNS_RRTYPE_A);
	dnsTail.usRequestClass = htons(DNS_RRTYPE_A);
	memcpy(byBuf + pDataPtr, &dnsTail, DNS_TAIL_SIZE);
	pDataPtr = pDataPtr + 4;

	iLastLen = pDataPtr;
	sendPkg->pData = (char *)malloc(iLastLen);
	memcpy(sendPkg->pData, byBuf, iLastLen);
	sendPkg->iDataLen = iLastLen;
}

static ParseRet processRequestReply(char *RecvBuf, const int BufLength, const char IsIpv6, const char *strdomain, char *outputip)
{
	int pDataPtr = 0;
	struct DNS_REQUEST_HEAD dnsHead;
	struct DNS_REQUEST_TAIL dnsTail;
	unsigned short usRedirectPtr = 0;
	unsigned char usanswers_len = 0;
	unsigned short usRecordType = 0;
	unsigned short usRecordClass = 0;
	unsigned short usTrueDataLen = 0;
	unsigned int ttl = 0;
	unsigned int iTrueIP = 0;
	struct in_addr userIP;
	char *pData = RecvBuf;
	struct in6_addr Temp;
	char *pstrbegin;
	char *pstrend;
	char domainstr[100] = {0}, domain[100] = {0};
	char answer_domain[128] = {0};
	int answerNum = 0;
	unsigned char found = 0;
	ParseRet ret = PARSE_DNS_SUCCESS;
	int answerCompFlag = 1; // dns response answers compress flag

	PP("processRequestReply begin");
	// DNS报头的装载
	memcpy((void *)&dnsHead, pData + pDataPtr, DNS_HEAD_SIZE); // pDataPtr=0
	dnsHead.usFlag.allFlag = ntohs(dnsHead.usFlag.allFlag);
	dnsHead.usQuestions = ntohs(dnsHead.usQuestions);
	dnsHead.usAnswer = ntohs(dnsHead.usAnswer);
	dnsHead.usAuthority = ntohs(dnsHead.usAuthority);
	dnsHead.usAdditional = ntohs(dnsHead.usAdditional);

	if (SendSessionID == dnsHead.usSessionID)
	{
		if (DMP_DNS_RESPONSE != dnsHead.usFlag.bits.Response)
		{
			PP("[typing] fun=%s allFlag=0x%x", __FUNCTION__, dnsHead.usFlag.allFlag);
			return ret;
		}

		if (DMP_ERROR_ZERO != dnsHead.usFlag.bits.Reply_code)
		{
			PP("[typing] fun=%s allFlag=0x%x Reply_code=0x%x", __FUNCTION__, dnsHead.usFlag.allFlag, dnsHead.usFlag.bits.Reply_code);
			return PARSE_DNS_ERROR;
		}

		if (0 == dnsHead.usAnswer)
		{
			PP("[typing] fun=%s allFlag=0x%x dnsHead.usAnswer=0", __FUNCTION__, dnsHead.usFlag.allFlag);
			return PARSE_DNS_ERROR;
		}
	}

	pDataPtr = pDataPtr + DNS_HEAD_SIZE; // pDataPtr=12
	pstrbegin = pData + pDataPtr;

	// body,temporary jump to End '\0'
	while ((pDataPtr < BufLength) && (0X00 != *(pData + pDataPtr)))
	{
		++pDataPtr;
	}

	++pDataPtr;
	pstrend = pData + pDataPtr;
	memcpy((void *)&domainstr, pstrbegin, (pstrend - pstrbegin));

	if (domainstr[0] != 0)
	{
		int i = 0, j, c, z = 0;
		while ((domainstr[i] != 0) && (i < sizeof(domainstr)) && (z < sizeof(domainstr)))
		{
			z++; // 防止死循环
			c = domainstr[i];
			for (j = 0; j < c; j++)
			{
				domain[i + j] = domainstr[i + j + 1];
			}
			if (domainstr[i + c + 1] != 0)
			{
				domain[i + j] = '.';
			}
			i = i + c + 1;
		}
	}
	PP("domain=%s", domain);
	// 报文身体
	memcpy((void *)&dnsTail, pData + pDataPtr, DNS_TAIL_SIZE);
	pDataPtr = pDataPtr + DNS_TAIL_SIZE;

	while (pDataPtr < BufLength)
	{
		// DomainStr，重定向报文域名 2个字节
		// 一般为C00C，二进制为1100000000001100
		// 最开始2Bit为11，剩下的Bit构成一个指针，指向DomainStr
		memcpy(&usRedirectPtr, pData + pDataPtr, 2);
		usRedirectPtr = ntohs(usRedirectPtr);
		int m = 0;
		while ((pDataPtr < BufLength) && (0xC000 != (usRedirectPtr & 0xC000)) && m < strlen(domainstr))
		{
			memcpy(&usanswers_len, pData + pDataPtr, 1);
			pDataPtr += 1;

			memcpy(&answer_domain[m], pData + pDataPtr, usanswers_len);
			pDataPtr += usanswers_len;
			m = m + usanswers_len;
			if (!strcasecmp(answer_domain, strdomain))
			{
				found = 1;
				break;
			}

			if (0 != m)
			{
				answer_domain[m] = '.';
			}
			m += 1;

			memcpy(&usRedirectPtr, pData + pDataPtr, 2);
			usRedirectPtr = ntohs(usRedirectPtr);
		}

		if (0xC000 == (usRedirectPtr & 0xC000))
		{
			answerCompFlag = 1;
		}
		else
		{
			answerCompFlag = 0;
		}

		if (answerCompFlag)
		{
			pDataPtr = pDataPtr + 2;

			if ((0 == answerNum) && (0xC000 == (usRedirectPtr & 0xC000)))
			{
				found = 1;
			}

			if (answerNum >= dnsHead.usAnswer)
			{
				if (0xC000 == (usRedirectPtr & 0xC000))
				{
					found = 1;
				}
				else
				{
					found = 0;
				}
			}
		}
		else
		{
			// name body,temporary jump to End '\0'
			while ((pDataPtr < BufLength) && (0X00 != *(pData + pDataPtr)))
			{
				++pDataPtr;
			}
			++pDataPtr;
			found = 1;
		}

		// 类型,2 BYTES
		memcpy(&usRecordType, pData + pDataPtr, 2);
		pDataPtr = pDataPtr + 2;
		usRecordType = ntohs(usRecordType);

		// 类,2 BYTES
		memcpy(&usRecordClass, pData + pDataPtr, 2);
		pDataPtr = pDataPtr + 2;
		usRecordClass = ntohs(usRecordClass);

		// TTL,4 BYTES
		memcpy(&ttl, pData + pDataPtr, 4);
		pDataPtr = pDataPtr + 4;

		// 后续真实数据长度，2 BYTES
		memcpy(&usTrueDataLen, pData + pDataPtr, 2);
		pDataPtr = pDataPtr + 2;
		usTrueDataLen = ntohs(usTrueDataLen);

		if (IsIpv6)
		{
			if ((DNS_RRTYPE_AAAA == usRecordType) && (!strcmp(strdomain, domain)) && (1 == found))
			{
				if (usTrueDataLen == 16)
				{
					memcpy(&Temp.s6_addr, pData + pDataPtr, usTrueDataLen);

					// inet_ntop(AF_INET6, &Temp.s6_addr, (char *)&outputip, sizeof(outputip));
					// can't use memcpy,the end token of string is mixed when there is 2 or more address and the first is longer than the second
					inet_ntop(AF_INET6, &Temp, outputip, 49);

					PP("outputip=%s--------------", outputip);
					break;
				}
			}
		}
		else
		{
			if ((DNS_RRTYPE_A == usRecordType) && (!strcmp(strdomain, domain)) && (1 == found))
			{
				if (usTrueDataLen == 4)
				{
					memcpy(&iTrueIP, pData + pDataPtr, 4);
					userIP.s_addr = iTrueIP;

					// memcpy(outputip, inet_ntoa(userIP), strlen(inet_ntoa(userIP)));
					// can't use memcpy,the end token of string is mixed when there is 2 or more address and the second is longer than first
					inet_ntop(AF_INET, &userIP, outputip, 16);

					PP("outputip=%s---------------------", outputip);
					break;
				}
			}
		}
		pDataPtr = pDataPtr + usTrueDataLen;
		answerNum++;
	}

	return ret;
}

ParseRet ifnameGetIPv6addr(const char *ifname, char islinkaddr, char *hoststr)
{
	struct ifaddrs *ifaddr, *ifa;
	int family;
	char *clientip = NULL;

	PP("ifnameGetIPv6addr begin ifname=%s", ifname);

	if ((ifname == NULL) || (hoststr == NULL))
	{
		return GET_IFNAMEIP_ERROR;
	}

	if (getifaddrs(&ifaddr) == -1)
	{
		PP("getifaddrs");
		return GET_IFNAMEIP_ERROR;
	}

	for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
	{
		if (ifa->ifa_addr == NULL)
			continue;

		family = ifa->ifa_addr->sa_family;

		if (strcmp(ifa->ifa_name, ifname) != 0)
		{
			continue;
		}

		if (family == AF_INET6)
		{
			if (ifa->ifa_flags & IFF_UP)
			{
				PP("the interface status is UP");
			}
			else
			{
				PP("the interface status is DOWN");
				freeifaddrs(ifaddr);
				return GET_IFNAMEIP_ERROR;
			}

			if (((islinkaddr) && ((((struct sockaddr_in6 *)ifa->ifa_addr)->sin6_addr.s6_addr32[0] & htonl(0xFFC00000)) == htonl(0xFE800000))) || ((!islinkaddr) && ((((struct sockaddr_in6 *)ifa->ifa_addr)->sin6_addr.s6_addr32[0] & htonl(0xFFC00000)) != htonl(0xFE800000))))
			{
				inet_ntop(AF_INET6, &((struct sockaddr_in6 *)ifa->ifa_addr)->sin6_addr, hoststr, 50);
				clientip = hoststr;
				PP("host: %s", hoststr);
				break;
			}
		}
	}

	freeifaddrs(ifaddr);

	if (!clientip)
	{
		PP("the interface: %s is error", ifname);
		return GET_IFNAMEIP_ERROR;
	}
	return GET_IFNAMEIP_SUCCESS;
}
ParseRet pParaseIpv4Domain(const char *ifname, const char *strdomain, const char *dnsserver, char *domainIp)
{
	struct sockaddr_in cli_addr;
	struct sockaddr_in serv_addr, back_serv_addr;
	int sockfd;
	char tmpBuf[128] = {0}, dnsSecondary[64] = {0};
	char *separator = NULL;
	// char haveBackDns = 0;
	char hoststr[50] = {0};
	int sockopt;

	PP("ParaseIpv4Domain begin :%s,requeset dns:%s,dns server:%s", ifname, strdomain, dnsserver);
	if ((NULL == domainIp) || (NULL == ifname) || (NULL == strdomain) || (NULL == dnsserver))
	{
		PP("Parameters can not be NULL");
		return PARAMETERS_IS_NULL;
	}

	bzero(&serv_addr, sizeof(struct sockaddr_in));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(53); /* 53 */
	if (ifnameGetIPv4addr(ifname, hoststr) != GET_IFNAMEIP_SUCCESS)
	{
		return GET_IFNAMEIP_ERROR;
	}
	strncpy(tmpBuf, dnsserver, sizeof(tmpBuf));
	separator = strstr(tmpBuf, ",");

	if (separator != NULL)
	{
		/* break the string into two strings */
		*separator = 0;
		separator++;
		while ((isspace0(*separator)) && (*separator != 0))
		{
			/* skip white space after comma */
			separator++;
		}

		strcpy(dnsSecondary, separator);
		PP("dnsSecondary=%s", dnsSecondary);
	}

	if (inet_aton(tmpBuf, &serv_addr.sin_addr) < 0)
	{
		PP("dnsserver error");
		return TRANS_DNSSERVER_ERROR;
	}

	if (dnsSecondary[0] != 0)
	{
		// haveBackDns = 1;
		bzero(&back_serv_addr, sizeof(struct sockaddr_in));
		back_serv_addr.sin_family = AF_INET;
		back_serv_addr.sin_port = htons(53); /* 53 */

		if (inet_aton(dnsSecondary, &back_serv_addr.sin_addr) < 0)
		{
			PP("dnsserver error");
			return TRANS_DNSSERVER_ERROR;
		}
	}

	// sockfd=socket(AF_INET, SOCK_DGRAM, 0);
	sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sockfd < 0)
	{
		fprintf(stderr, "Socket Error:%s\n", strerror(errno));
		return SOCKET_CREATE_FAIL;
	}

	bzero(&cli_addr, sizeof(struct sockaddr_in));
	cli_addr.sin_family = AF_INET;
	cli_addr.sin_port = htons(0);

	if (inet_aton(hoststr, &cli_addr.sin_addr) < 0)
	{
		PP("client Ip error");
		close(sockfd);
		return CLIENT_IP_ERROR;
	}

	sockopt = 1;
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (void *)&sockopt, sizeof(sockopt)) == -1)
	{
		PP("fail to set sock opt SO_REUSEADDR");
	}

	if (bind(sockfd, (struct sockaddr *)&cli_addr, sizeof(struct sockaddr_in)) < 0)
	{
		PP("bind");
		close(sockfd);
		return BIND_IP_ERROR;
	}

	getPackageRequestIP(&sendPkg, strdomain, 0);

	if (sendto(sockfd, sendPkg.pData, sendPkg.iDataLen, 0, (struct sockaddr *)&serv_addr, sizeof(struct sockaddr_in)) < 0)
	{
		PP("sendto");
		PP("send dns request error!");
		free(sendPkg.pData);
		sendPkg.pData = NULL;
		close(sockfd);
		return SEND_DNS_REQUEST_FAIL;
	}
	else
	{
		PP("send dns request success!");
		return PARSE_DNS_SUCCESS;
	}
}

ParseRet ParaseIpv6Domain(const char *ifname, const char *strdomain, const char *dnsserver, char *domainIp, const char IsIpv6)
{
	struct sockaddr_in6 cli_addr;
	struct sockaddr_in6 serv_addr, back_serv_addr;
	int ret;
	int sockfd, m_iRecvDataLen, maxfd;
	char m_szRecvBuf[1024];
	fd_set rfds;
	struct timeval timeout;
	char SwitchBackDns = 0;
	char tmpBuf[128] = {0}, dnsSecondary[64] = {0};
	char *separator = NULL;
	char haveBackDns = 0;
	char hoststr[50] = {0};
	int trytime = 0, sockopt;
	int addrlen = 0;
	char outputip[128] = {0};
	int binderr = 0;

	PP("begin");
	if ((NULL == domainIp) || (NULL == ifname) || (NULL == strdomain) || (NULL == dnsserver))
	{
		PP("Parameters can not be NULL");
		return PARAMETERS_IS_NULL;
	}

	bzero(&serv_addr, sizeof(struct sockaddr_in6));
	serv_addr.sin6_family = AF_INET6;
	serv_addr.sin6_port = htons(53); /* 53 */
	/*addb by hhr, slave have no internet address*/
	if (ifnameGetIPv6addr(ifname, 0, hoststr) != GET_IFNAMEIP_SUCCESS)
	{
		return GET_IFNAMEIP_ERROR;
	}

	strncpy(tmpBuf, dnsserver, sizeof(tmpBuf));
	separator = strstr(tmpBuf, ",");

	if (separator != NULL)
	{
		/* break the string into two strings */
		*separator = 0;
		separator++;
		while ((isspace0(*separator)) && (*separator != 0))
		{
			/* skip white space after comma */
			separator++;
		}

		strcpy(dnsSecondary, separator);
		PP("dnsSecondary=%s", dnsSecondary);
	}

	if (inet_pton(AF_INET6, tmpBuf, &serv_addr.sin6_addr) < 0)
	{
		PP("dnsserver error");
		return TRANS_DNSSERVER_ERROR;
	}

	if (dnsSecondary[0] != 0)
	{
		haveBackDns = 1;
		bzero(&back_serv_addr, sizeof(struct sockaddr_in6));
		back_serv_addr.sin6_family = AF_INET6;
		back_serv_addr.sin6_port = htons(53); /* 53 */

		if (inet_pton(AF_INET6, dnsSecondary, &back_serv_addr.sin6_addr) < 0)
		{
			PP("dnsserver error");
			return TRANS_DNSSERVER_ERROR;
		}
	}

	sockfd = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
	if (sockfd < 0)
	{
		fprintf(stderr, "Socket Error:%s\n", strerror(errno));
		return SOCKET_CREATE_FAIL;
	}

	bzero(&cli_addr, sizeof(struct sockaddr_in6));
	cli_addr.sin6_family = AF_INET6;
	cli_addr.sin6_port = htons(0);

	if (inet_pton(AF_INET6, hoststr, &cli_addr.sin6_addr) < 0)
	{
		PP("client Ip error");
		close(sockfd);
		return CLIENT_IP_ERROR;
	}

	sockopt = 1;
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (void *)&sockopt, sizeof(sockopt)) == -1)
	{
		PP("fail to set sock opt SO_REUSEADDR");
	}

	do
	{
		if (bind(sockfd, (struct sockaddr *)&cli_addr, sizeof(struct sockaddr_in6)) < 0)
		{
			binderr++;
			PP("bind error: %s , hoststr:[%s], binderr:[%d]", strerror(errno), hoststr, binderr);
			if (binderr > 2)
			{
				close(sockfd);
				return BIND_IP_ERROR;
			}
			sleep(1 + binderr * 2);
		}
		else
		{
			break;
		}
	} while (binderr < 2);

	getPackageRequestIP(&sendPkg, strdomain, IsIpv6);

	if ((ret = sendto(sockfd, sendPkg.pData, sendPkg.iDataLen, 0, (struct sockaddr *)&serv_addr, sizeof(struct sockaddr_in6))) < 0)
	{
		PP("send dns request error: %s", strerror(errno));
		free(sendPkg.pData);
		sendPkg.pData = NULL;
		close(sockfd);
		return SEND_DNS_REQUEST_FAIL;
	}
	else
	{
		PP("send dns request success!");
	}

	while (1)
	{
		PP("begin");
		// optimize dns time
		if (0 == trytime)
		{
			timeout.tv_sec = 1;
			timeout.tv_usec = 0;
		}
		else if (1 == trytime)
		{
			timeout.tv_sec = 2;
			timeout.tv_usec = 0;
		}
		else if (2 == trytime)
		{
			timeout.tv_sec = 3;
			timeout.tv_usec = 0;
		}
		else
		{
			timeout.tv_sec = 5;
			timeout.tv_usec = 0;
		}
		FD_ZERO(&rfds);
		FD_SET(sockfd, &rfds);
		maxfd = sockfd + 1;

		switch (select(maxfd, &rfds, NULL, NULL, &timeout))
		{
		case -1:
			free(sendPkg.pData);
			sendPkg.pData = NULL;
			close(sockfd);
			return READ_SOCKET_FAIL;
			break;
		case 0:
			PP("timeout ");
			if (trytime < MAX_TRY_TIME)
			{
				if (SwitchBackDns)
				{
					ret = sendto(sockfd, sendPkg.pData, sendPkg.iDataLen, 0, (struct sockaddr *)&back_serv_addr, sizeof(struct sockaddr_in6));
				}
				else
				{
					ret = sendto(sockfd, sendPkg.pData, sendPkg.iDataLen, 0, (struct sockaddr *)&serv_addr, sizeof(struct sockaddr_in6));
				}

				if (ret < 0)
				{
					PP("send dns request error!");
					free(sendPkg.pData);
					sendPkg.pData = NULL;
					close(sockfd);
					return SEND_DNS_REQUEST_FAIL;
				}
				else
				{
					trytime++;
					PP("send dns request success!");
					break;
				}
			}

			if (trytime >= MAX_TRY_TIME)
			{
				if ((haveBackDns) && (!SwitchBackDns))
				{
					SwitchBackDns = 1;
					// optimize dns time
					ret = sendto(sockfd, sendPkg.pData, sendPkg.iDataLen, 0, (struct sockaddr *)&back_serv_addr, sizeof(struct sockaddr_in6));
					if (ret < 0)
					{
						PP("send dns request error!");
						free(sendPkg.pData);
						sendPkg.pData = NULL;
						close(sockfd);
						return SEND_DNS_REQUEST_FAIL;
					}
					else
					{
						trytime = 0;
						PP("send dns request success!");
						break;
					}
				}
				free(sendPkg.pData);
				sendPkg.pData = NULL;
				close(sockfd);
				return SEND_SUCCESS;
			}
			break;
		default:
			if (FD_ISSET(sockfd, &rfds))
			{
				ParseRet ret2 = PARSE_DNS_SUCCESS;

				addrlen = sizeof(struct sockaddr_in6);
				m_iRecvDataLen = recvfrom(sockfd, &m_szRecvBuf, 1024, 0, (struct sockaddr *)&fromv6, (socklen_t *)&addrlen);

				PP("-----------------fromv6.sin6_ipaddr:" NIP6_FMT "\n", NIP6(fromv6.sin6_addr));
				PP("-----------------serv_addr.sin6_ipaddr:" NIP6_FMT "\n", NIP6(serv_addr.sin6_addr));

				if (!memcmp(fromv6.sin6_addr.s6_addr, serv_addr.sin6_addr.s6_addr, 16) || ((SwitchBackDns) && !memcmp(fromv6.sin6_addr.s6_addr, back_serv_addr.sin6_addr.s6_addr, 16)))
				{
					PP("m_iRecvDataLen=%d", m_iRecvDataLen);

					ret2 = processRequestReply((char *)&m_szRecvBuf, m_iRecvDataLen, IsIpv6, strdomain, outputip);

					if (PARSE_DNS_SUCCESS != ret2)
					{
						if (haveBackDns)
						{
							if (SwitchBackDns)
							{
								free(sendPkg.pData);
								sendPkg.pData = NULL;
								close(sockfd);
								return ret2;
							}
							else
							{
								SwitchBackDns = 1;
								trytime = 0;
							}
						}
						else
						{
							free(sendPkg.pData);
							sendPkg.pData = NULL;
							close(sockfd);
							return ret2;
						}
					}

					if (outputip[0] != 0)
					{
						strcpy(domainIp, outputip);
						free(sendPkg.pData);
						sendPkg.pData = NULL;
						close(sockfd);
						return PARSE_DNS_SUCCESS;
					}
				}
			}
			break;
		}
	}
}

char IsIpv6Addr(char *str)
{
	int i;
	PP("IsIpv6Addr begin");

	for (i = 0; i < strlen(str); i++)
	{
		if (((str[i] >= '0') && (str[i] <= '9')) || (str[i] == ':') || ((str[i] >= 'a') && (str[i] <= 'f')) || ((str[i] >= 'A') && (str[i] <= 'F')))
		{
			continue;
		}
		else
		{
			return 0;
		}
	}
	return 1;
}
static int ifnameGetIPv4addr1(const char *ifname, char *hoststr)
{
	int ret = -1;
	struct ifreq ifr;
	struct sockaddr_in *sin;
	int sock;
	char *p = NULL;

	PP("ifnameGetIPv4addr1 ifname:%s", ifname);
	sock = socket(AF_INET, SOCK_DGRAM, 0);
	strncpy(ifr.ifr_name, ifname, sizeof(ifr.ifr_name));

	if (0 > (ret = ioctl(sock, SIOCGIFADDR, &ifr)))
	{
		printf("ifnameGetIPv4addr1 ioctl get SIOCGIFADDR failed!!\n");
		close(sock);
		return -1;
	}
	close(sock);

	sin = (struct sockaddr_in *)&(ifr.ifr_addr);

	if (NULL == (p = inet_ntoa(sin->sin_addr)))
	{
		printf("ifnameGetIPv4addr1 find p null!!\n");
		return -1;
	}

	snprintf(hoststr, 1024, "%s", p);
	PP("ifnameGetIPv4addr1 success: %s", hoststr);

	return ret;
}

ParseRet ifnameGetIPv4addr(const char *ifname, char *hoststr)
{
	if (ifnameGetIPv4addr1(ifname, hoststr) != -1)
	{
		return GET_IFNAMEIP_SUCCESS;
	}
	else
	{
		struct ifaddrs *ifaddr, *ifa;
		int family;
		char *p = NULL;

		PP("ifnameGetIPv4addr begin ifname=%s", ifname);
		if ((ifname == NULL) || (hoststr == NULL))
		{
			return GET_IFNAMEIP_ERROR;
		}

		if (getifaddrs(&ifaddr) == -1)
		{
			PP("getifaddrs");
			return GET_IFNAMEIP_ERROR;
		}
		for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
		{
			if (ifa->ifa_addr == NULL)
				continue;

			family = ifa->ifa_addr->sa_family;

			if (strcmp(ifa->ifa_name, ifname) != 0)
			{
				continue;
			}

			if (family == AF_INET)
			{
				if (ifa->ifa_flags & IFF_UP)
				{
					PP("the interface status is UP");
				}
				else
				{
					PP("the interface status is DOWN");
					freeifaddrs(ifaddr);
					return GET_IFNAMEIP_ERROR;
				}

				p = inet_ntoa(((struct sockaddr_in *)ifa->ifa_addr)->sin_addr);
				snprintf(hoststr, 1024, "%s", p);
				PP("host: %s", hoststr);
				break;
			}
		}

		freeifaddrs(ifaddr);

		if (!p)
		{
			PP("the interface: %s is error", ifname);
			return GET_IFNAMEIP_ERROR;
		}
		return GET_IFNAMEIP_SUCCESS;
	}
}

ParseRet ParaseIpv4Domain(const char *ifname, const char *strdomain, const char *dnsserver, char *domainIp)
{
	struct sockaddr_in cli_addr;
	struct sockaddr_in serv_addr, back_serv_addr, from;
	int ret;
	int sockfd, m_iRecvDataLen, maxfd;
	char m_szRecvBuf[1024];
	fd_set rfds;
	struct timeval timeout;
	char SwitchBackDns = 0;
	char tmpBuf[128] = {0}, dnsSecondary[64] = {0};
	char *separator = NULL;
	char haveBackDns = 0;
	char hoststr[50] = {0};
	int trytime = 0, sockopt;
	int addrlen = 0;
	char outputip[128] = {0};

	PP("ParaseIpv4Domain begin :%s,requeset dns:%s,dns server:%s", ifname, strdomain, dnsserver);
	if ((NULL == domainIp) || (NULL == ifname) || (NULL == strdomain) || (NULL == dnsserver))
	{
		PP("Parameters can not be NULL");
		return PARAMETERS_IS_NULL;
	}

	bzero(&serv_addr, sizeof(struct sockaddr_in));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(53); /* 53 */
	/*addb by hhr, slave have no internet address*/
	if (ifnameGetIPv4addr(ifname, hoststr) != GET_IFNAMEIP_SUCCESS)
	{
		return GET_IFNAMEIP_ERROR;
	}
	strncpy(tmpBuf, dnsserver, sizeof(tmpBuf));
	separator = strstr(tmpBuf, ",");

	if (separator != NULL)
	{
		/* break the string into two strings */
		*separator = 0;
		separator++;
		while ((isspace0(*separator)) && (*separator != 0))
		{
			/* skip white space after comma */
			separator++;
		}

		strcpy(dnsSecondary, separator);
		PP("dnsSecondary=%s", dnsSecondary);
	}

	if (inet_aton(tmpBuf, &serv_addr.sin_addr) < 0)
	{
		PP("dnsserver error");
		return TRANS_DNSSERVER_ERROR;
	}

	if (dnsSecondary[0] != 0)
	{
		haveBackDns = 1;
		bzero(&back_serv_addr, sizeof(struct sockaddr_in));
		back_serv_addr.sin_family = AF_INET;
		back_serv_addr.sin_port = htons(53); /* 53 */

		if (inet_aton(dnsSecondary, &back_serv_addr.sin_addr) < 0)
		{
			PP("dnsserver error");
			return TRANS_DNSSERVER_ERROR;
		}
	}

	/* 建立 sockfd描述符 */
	// sockfd=socket(AF_INET, SOCK_DGRAM, 0);
	sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sockfd < 0)
	{
		fprintf(stderr, "Socket Error:%s\n", strerror(errno));
		return SOCKET_CREATE_FAIL;
	}

	bzero(&cli_addr, sizeof(struct sockaddr_in));
	cli_addr.sin_family = AF_INET;
	cli_addr.sin_port = htons(0);

	if (inet_aton(hoststr, &cli_addr.sin_addr) < 0)
	{
		PP("client Ip error");
		close(sockfd);
		return CLIENT_IP_ERROR;
	}

	sockopt = 1;
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (void *)&sockopt, sizeof(sockopt)) == -1)
	{
		PP("fail to set sock opt SO_REUSEADDR");
	}

	if (bind(sockfd, (struct sockaddr *)&cli_addr, sizeof(struct sockaddr_in)) < 0)
	{
		PP("bind");
		close(sockfd);
		return BIND_IP_ERROR;
	}

	getPackageRequestIP(&sendPkg, strdomain, 0);

	if ((ret = sendto(sockfd, sendPkg.pData, sendPkg.iDataLen, 0, (struct sockaddr *)&serv_addr, sizeof(struct sockaddr_in))) < 0)
	{
		PP("sendto");
		PP("send dns request error!");
		free(sendPkg.pData);
		sendPkg.pData = NULL;
		close(sockfd);
		return SEND_DNS_REQUEST_FAIL;
	}
	else
	{
		PP("send dns request success!");
	}

	while (1)
	{
		PP("begin");
		// optimize dns time
		if (0 == trytime)
		{
			timeout.tv_sec = 1;
			timeout.tv_usec = 0;
		}
		else if (1 == trytime)
		{
			timeout.tv_sec = 2;
			timeout.tv_usec = 0;
		}
		else if (2 == trytime)
		{
			timeout.tv_sec = 3;
			timeout.tv_usec = 0;
		}
		else
		{
			timeout.tv_sec = 5;
			timeout.tv_usec = 0;
		}
		FD_ZERO(&rfds);
		FD_SET(sockfd, &rfds);
		maxfd = sockfd + 1;

		switch (select(maxfd, &rfds, NULL, NULL, &timeout))
		{
		case -1:
			free(sendPkg.pData);
			sendPkg.pData = NULL;
			close(sockfd);
			return READ_SOCKET_FAIL;
			break;
		case 0:
			PP("timeout ");
			if (trytime < MAX_TRY_TIME)
			{
				if (SwitchBackDns)
				{
					ret = sendto(sockfd, sendPkg.pData, sendPkg.iDataLen, 0, (struct sockaddr *)&back_serv_addr, sizeof(struct sockaddr_in));
				}
				else
				{
					ret = sendto(sockfd, sendPkg.pData, sendPkg.iDataLen, 0, (struct sockaddr *)&serv_addr, sizeof(struct sockaddr_in));
				}

				if (ret < 0)
				{
					PP("send dns request error!");
					free(sendPkg.pData);
					sendPkg.pData = NULL;
					close(sockfd);
					return SEND_DNS_REQUEST_FAIL;
				}
				else
				{
					trytime++;
					PP("send dns request success!");
					break;
				}
			}

			if (trytime >= MAX_TRY_TIME)
			{
				if ((haveBackDns) && (!SwitchBackDns))
				{
					SwitchBackDns = 1;
					// optimize dns time
					ret = sendto(sockfd, sendPkg.pData, sendPkg.iDataLen, 0, (struct sockaddr *)&back_serv_addr, sizeof(struct sockaddr_in));

					if (ret < 0)
					{
						PP("send dns request error!");
						free(sendPkg.pData);
						sendPkg.pData = NULL;
						close(sockfd);
						return SEND_DNS_REQUEST_FAIL;
					}
					else
					{
						trytime = 0;
						PP("send dns request success!");
						break;
					}
				}
				free(sendPkg.pData);
				sendPkg.pData = NULL;
				close(sockfd);
				return SEND_SUCCESS;
			}
			break;
		default:
			if (FD_ISSET(sockfd, &rfds))
			{
				ParseRet ret2 = PARSE_DNS_SUCCESS;

				addrlen = sizeof(struct sockaddr);
				m_iRecvDataLen = recvfrom(sockfd, &m_szRecvBuf, 1024, 0, (struct sockaddr *)&from, (socklen_t *)&addrlen);
				if ((from.sin_addr.s_addr == serv_addr.sin_addr.s_addr) || ((SwitchBackDns) && (from.sin_addr.s_addr == back_serv_addr.sin_addr.s_addr)))
				{
					PP("m_iRecvDataLen=%d,SwitchBackDns=%d", m_iRecvDataLen, SwitchBackDns);

					ret2 = processRequestReply((char *)&m_szRecvBuf, m_iRecvDataLen, 0, strdomain, outputip);

					if (PARSE_DNS_SUCCESS != ret2)
					{
						if (haveBackDns)
						{
							if (SwitchBackDns)
							{
								free(sendPkg.pData);
								sendPkg.pData = NULL;
								close(sockfd);
								return ret2;
							}
							else
							{
								SwitchBackDns = 1;
								trytime = 0;
							}
						}
						else
						{
							free(sendPkg.pData);
							sendPkg.pData = NULL;
							close(sockfd);
							return ret2;
						}
					}

					if (outputip[0] != 0)
					{
						strcpy(domainIp, outputip);
						free(sendPkg.pData);
						sendPkg.pData = NULL;
						close(sockfd);
						return PARSE_DNS_SUCCESS;
					}
				}
			}
			break;
		}
	}
}