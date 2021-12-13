#include <sys/socket.h>
#include <netinet/in.h>
#include<arpa/inet.h>
#include <net/if.h>
#include <sys/types.h>
#include <netdb.h>
#include <sys/socket.h>

static int bua_commGetAddrInfo(const char *url, unsigned int *hostIp, unsigned int *hostPort)
{
	char *token = NULL;
	char regURL[64] = {0};
	char urlStr[256] = {0};

	strncpy(urlStr, url, sizeof(urlStr));
	token = strtok(urlStr, "/");
	token = strtok(NULL, "/");
	printf("token = %s", token);
	token = strtok(token, ":");
	strncpy(regURL, token, sizeof(regURL));
	token = strtok(NULL, ":");
	if (NULL != token)
	{
		*hostPort = atoi(token);
	}
	printf("regURL=%s\n", regURL);
	struct hostent *hptr;
	struct in_addr inaddr;
	if ((hptr = gethostbyname(regURL)) == NULL)
	{
		printf("gethostbyname error for host:%s\n", regURL);
		return -1;
	}

	*hostIp = *((unsigned int *)hptr->h_addr_list[0]);
	inaddr.s_addr = *hostIp;
	printf("url = %s *hostIp = %s:%d regURL = %s\n", url, inet_ntoa(inaddr), *hostPort,  regURL);
	return 0;
}

int BUA_commConnect(const char *url)
{
	struct sockaddr serverAddr;
	struct sockaddr clientAddr;
	int sk = -1;
	int af = 0;
	struct timeval tv = {1, 0};
	struct ifreq ifr;
	int i = 0;
	int ret = 0;
	unsigned int hostIp = 0;
	unsigned int port = 80;

	memset(&serverAddr, 0, sizeof(serverAddr));
	memset(&clientAddr, 0, sizeof(clientAddr));

	ret = bua_commGetAddrInfo(url, &hostIp, &port);
	printf("url=%s, &hostIp = %d\n", url, hostIp);
	port = (0 == port) ? 80 : port;
	if (0 != ret)
	{
		printf("platform dns resolve fail! ret = %d", ret);
		return ret;
	}
	printf("analyzeDnsInfo result:ip = %x", hostIp);

#if 1
	af = AF_INET;
	((struct sockaddr_in *)&serverAddr)->sin_addr.s_addr = hostIp;
	((struct sockaddr_in *)&serverAddr)->sin_port = htons(port);
	((struct sockaddr_in *)&serverAddr)->sin_family = AF_INET;
#else
	if (inet_pton(AF_INET6, hostIp, (void *) & (((struct sockaddr_in6 *)&serverAddr)->sin6_addr)))
	{
		af = AF_INET6;
		((struct sockaddr_in6 *)&serverAddr)->sin6_port = htons(80);
		((struct sockaddr_in6 *)&serverAddr)->sin6_family = AF_INET6;
	}
	else if (inet_pton(AF_INET, hostIp, (void *) & (((struct sockaddr_in *)&serverAddr)->sin_addr)))
	{
		af = AF_INET;
		((struct sockaddr_in *)&serverAddr)->sin_port = htons(80);
		((struct sockaddr_in *)&serverAddr)->sin_family = AF_INET;
	}
#endif

	sk = socket(af, SOCK_STREAM, 0);
	if (sk <= 0)
	{
		printf("create sk = %d ,errno = %d,resion: %s\n", sk, errno, strerror(errno));
		return -1;
	}
	printf("sk = %d", sk);

	if (af == AF_INET6)
	{
		if (setsockopt(sk, IPPROTO_IPV6, IPV6_V6ONLY, (void *) &i, sizeof(i)) < 0)
		{
			printf("setsockopt IPV6_V6ONLY");
			return -1;
		}
	}

	if (setsockopt(sk, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(struct timeval)) < 0)
	{
		printf("Fail to set sock opt! %s", strerror(errno));
		return -1;
	}
	if (1 < util_strlen(g_wanIfName))
	{
		strncpy(ifr.ifr_name, g_wanIfName, sizeof(ifr.ifr_name));
		if (setsockopt(sk, SOL_SOCKET, SO_BINDTODEVICE, &ifr, sizeof(ifr)) < 0)
		{
			printf("bind socket to device error! %s", strerror(errno));
		}

		if (connect(sk, (struct sockaddr *)&serverAddr,
		            af == AF_INET6 ? sizeof(struct sockaddr_in6) : sizeof(struct sockaddr)) < 0)
		{
			if (EINPROGRESS != errno)
			{
				printf("connect sk = %d ,errno = %d,reason: %s\n", sk, errno, strerror(errno));
				close(sk);
				return -1;
			}
		}

		printf("mmc connect to url : %s port : %d success", url, 80);
		return sk;
	}
	else
	{
		printf("g_wanIfName is NULl");
		return -1;
	}
}
