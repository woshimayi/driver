#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int fun(void)
{
	return 0;
}

int fun1(char *conse)
{
	printf("2222222222222  %s\n", conse);
	return 0;
}


#define fun() fun1(__FUNCTION__)

int main()
{
	fun();
	return 0;
}


