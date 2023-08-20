#include <stdio.h>
#include <string.h>
#include <stdlib.h>


int fun(char *ifname, char *ipaddr)
{
	char ifn[] = "12345";
	char addr[] = "98765";
	strncpy(ifname, ifn, sizeof(ifn));
	strncpy(ipaddr, addr, sizeof(addr));
	return 0;
}

int fun1(char **ifname, char **ipaddr)
{
	char *ifn = "12345";
	char *addr = "98765";

	ifname = (char *)malloc(64);
	ipaddr = (char *)malloc(64);
	// strncpy(ifname, ifn, sizeof(ifn));
	// strncpy(ipaddr, addr, sizeof(addr));
	ifname = ifn;
	ipaddr = addr;
	printf("ifn = %s addr = %s", ifname, ipaddr);
	return 0;
}


int main(int argc, char const *argv[])
{
#if 0
	char ifn[64] = {0};
	char addr[64] = {0};
	fun(ifn, addr);
#elif 0
	char *ifn = malloc(64);
	char *addr = malloc(64);
	fun(ifn, addr);
#else
	char *ifn  = NULL;
	char *addr = NULL;
	fun1(&ifn, &addr);
	printf("ifn = %s addr = %s", ifn, addr);
	if (ifn)
		free(ifn);
	if (addr)
		free(addr);
#endif
	printf("ifn = %s addr = %s", ifn, addr);
	return 0;
}

