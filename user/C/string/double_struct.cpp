#include<stdio.h>
#include<string.h>
#include<stdlib.h>

typedef unsigned char UINT8;
typedef char UINT16;

typedef struct
{
	UINT8 Index;
	UINT16 Maxsta;
} _OamSvaSsid;

typedef struct
{
	UINT8 Item;
	_OamSvaSsid SsidMaxsta;
} _OamSvaMaxUserConnect;


int main()
{
	_OamSvaMaxUserConnect  oamSvaMaxUserConnect = {3, {2, 32}};

	char *endptr = NULL;
	char *str = "wl0.2";
	char str1[12] = "";

	strcpy(str1, str);

	printf("str1  =%s\n", str1);
	strtok(str1, ".");
	endptr = strtok(NULL, ".");

	printf("endptr = %s %d", endptr, atoi(endptr));

	return 0;
}

