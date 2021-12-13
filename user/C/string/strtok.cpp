#include<stdio.h>
#include<string.h>
#include<stdlib.h>

int main()
{
	char str[] = "10.188.107.1,180.168.255.118";
	char *p[2];
	char *buf = str;
	char  str1[4];
	int num = 0;
	p[0] = strtok(str, ",");
	p[1] = strtok(NULL, ",");

	printf("p0 = %s %d\n", p[0], sizeof(p[0]));
	printf("p1 = %s %d\n", p[1], sizeof(p[1]));

	char *k[4];

	k[0] = strtok(p[0], ".");
	k[1] = strtok(NULL, ".");
	k[2] = strtok(NULL, ".");
	k[3] = strtok(NULL, ".");

	printf("%x\n", atoi(k[0]));
	printf("%x\n", atoi(k[1]));
	printf("%x\n", atoi(k[2]));
	printf("%x\n", atoi(k[3]));

	sprintf(&str1[0], "%x\n", atoi(k[0]));
	sprintf(&str1[1], "%d\n", atoi(k[1]));
	sprintf(&str1[2], "%d\n", atoi(k[2]));
	sprintf(&str1[3], "%d\n", atoi(k[3]));
	printf("str = %s\n", &str1[0]);
	printf("%d\n", num);

	return 0;
}

