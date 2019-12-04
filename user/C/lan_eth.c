#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char* argv[])
{
	char lantmp[32] = "LAN1";
	char tmp[32] = "eth0";
	int i = 0;
	int n = 0;
	printf("lantmp = %s %d\n", argv[1], strlen(argv[1]));

	strncpy(tmp, argv[1], strlen(argv[1])+1);
		
	printf("tmp = %s\n", tmp);
	
	for(i = 0; i < strlen(tmp)+1; i++)
	{
	    tmp[i] = tolower(tmp[i]);
	}
		printf("tmp = %s\n", tmp);

//	sscanf(lantmp, "lan%d", &i);
//	sscanf(tmp, "eth%d", &n);
//	printf("lan = %d\neth = %d\n",  i, n);
	return 0;
}


