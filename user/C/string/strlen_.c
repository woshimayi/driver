#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

int main()
{
	char emptyStr[1] = {0};
	char *str = (char *)emptyStr;
	printf("%d\n", strlen(str));

	return 0;
}

