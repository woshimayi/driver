#include<stdio.h>
#include<string.h>
#include<stdlib.h>

int main()
{
	char str[] = "wl0";
	char str1[] = "wl0.1";
	char str2[] = "wl0.2";
	char str3[] = "wl0.3";
	char *p = NULL;
	p = strtok(str, ".");
	printf("p = %s\n", p);
	p = strtok(NULL, ".");
	printf("p = %d\n", atoi(p));

	if (p == NULL)
	{
		printf("%s\n", p);
	}

	return 0;
}

