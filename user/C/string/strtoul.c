#include<stdio.h>
#include<stdlib.h>
int main()
{
	int a;
	char pNum[] = "0x77";
	a = strtoul(pNum, 0, 0); //最后的0，表示自动识别pNum是几进制
	printf("%u\n", a);
	return 0;
}
