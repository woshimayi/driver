#include<stdio.h>
#include<string.h>
#include<stdlib.h>

#define PP printf("[%s:%d]\n", __FUNCTION__, __LINE__);

typedef void (*CALLFUN)(char*);

void CallPrintfText(CALLFUN fp, char * s)
{
	fp(s);
}

void PrintfText(char * s)
{
	printf(s);
}

int main()
{
	CallPrintfText(PrintfText, "Hello World\n");
	PP
	return 0;
}

