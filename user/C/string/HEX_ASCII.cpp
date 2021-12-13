#include<stdio.h>
#include<string.h>
#include<stdlib.h>
typedef char UINT8;
int main()
{
	UINT8 num[64] = {0x31, 0x5F, 0x49, 0x4E, 0x54, 0x45, 0x52, 0x4E, 0x45, 0x54, 0x5F, 0x42, 0x5F, 0x56, 0x49, 0x44, 0x5F, 0x31, 0x30, 0x30};
	char num1[64] = {0};
	sprintf(num1, "%s\n", num);
	printf("num1 = %s\n", num1);
	int i = 0;
	while (num != "\0")
	{
		printf("%x\n", num[i++]);
	}
	return 0;
}

