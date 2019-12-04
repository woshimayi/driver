#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main()
{
	char *val[128] = {0};
	int respBufSize = strlen(val[0]) + 1;
	char *respBuf = NULL;
	
//	strncpy(val[0], "sdfsds", "32");
	
//	printf("val = %s\n", val[0]);
	
	strncpy(respBuf, "sdsdsd", 32);
	
	printf("%s", &respBuf[0]);
	
	return 0;
}


