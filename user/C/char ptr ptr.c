#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main()
{
	char * str = "sdfsd";
	char **full = {0};
	*full = strdup(str);
	printf("%s", *full);
	return 0;
}


