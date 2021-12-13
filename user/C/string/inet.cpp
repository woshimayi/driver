#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>

int main(int argc, char **argv)
{
	const char *ip_str = "127.0.0.1";
	char *ip_res;
	in_addr_t addr_t;
	struct in_addr addr;

	//1.str -> binary
	inet_aton(ip_str, &addr);
	printf("inet_aton::%x\n", addr);                //1000007f

	addr_t = inet_addr(ip_str);
	printf("inet_addr::%x\n", addr_t);              //1000007f

	inet_pton(AF_INET, ip_str, (void *)&addr);      //1000007f
	printf("inet_pton::%x\n", addr);

	//2.binary -> str
	ip_res = inet_ntoa(addr);
	printf("inet_ntoa::%s\n", ip_res);              //127.0.0.1

	inet_ntop(AF_INET, &addr, ip_res, INET_ADDRSTRLEN);
	printf("inet_ntop::%s\n", ip_res);              //127.0.0.1

	return 0;
}

