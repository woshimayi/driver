#include <stdio.h>
#include <stdlib.h>
#include <string.h>


void fun1(char *str1)
{
	char * str = "aaaaaaa";
	strncpy(str1, str, sizeof(str));
	printf("fun1 str= %s \n", str1);
}

void fun(char *str1)
{
	fun1(str1);
	printf("fun str= %s \n", str1);
}

int main()
{
	char str1[128] = "sssssssss";
	printf("str1 =  %s\n", str1);
	fun(str1);
	printf("str1 = %s\n", str1);
	return 0;
}


