#include<stdio.h>
#include<string.h>
#include<stdlib.h>

typedef struct 
{
	char * str;
}abc;

typedef struct 
{
	char * str1;
}cde;

int main()
{
	abc abc = {"abc"};
	cde cde = {"cde"};
	void * dest = NULL;
	void * dest1 = NULL;
	(abc*)dest = abc;
	
	printf("dest->str = %s\n", dest1->str);
	return 0;
}

