#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main()
{
	char tempBuf[100] = "00000001";
	char value[100] = {0};
	char turnStr[100] = {0};
	unsigned long int getValue = 0;
	int i = 0;
	int getLine = strlen(tempBuf);
	
	
	for(i = 0; i < (getLine / 2 ); i++)
	{
		strncpy(turnStr, &tempBuf[2*i], 3);
		getValue = strtoul(turnStr, 0, 0);
		value[i] = (char)getValue;
	}
	printf("value = %s\n", value);
	printf("i = %d", i);
	return 0;
}


