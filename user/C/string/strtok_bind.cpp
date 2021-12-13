#include<stdio.h>
#include<string.h>
#include<stdlib.h>

int main()
{
	char str[] = "InternetGatewayDevice.LANDevice.1.LANEthernetInterfaceConfig.3";
	//,InternetGatewayDevice.LANDevice.1.LANEthernetInterfaceConfig.4";
	int i = 0;
	char *p[2];
	char *buf = str;
	while (NULL != (p[i] = strtok(buf, ",")))
	{
		i++;
		buf = NULL;
	}

	printf("p1 = %s\n", p[0]);
	printf("p1 = %s\n", p[1]);
	return 0;
}

