#include <stdio.h>
#include <string.h>
#include <stdlib.h>


int g = 0;

#define setbit(x,y) x|=(1<<y)
#define clrbit(x,y) x&=~(1<<y)
#define reversebit(x,y) x^=(1<<y)
#define getbit(x,y) ((x) >> (y)&1)

int main(int argc, char const *argv[])
{
	int a = 0xff;
	printf("a:\n");
	printf(" a\t%8x\n~a\t%8x\n\n", a, 0xff & ~a);

	printf("÷√ 1");
	a = 0x0;
	a |= 0xff & 1 << 3;
	printf("a:\n");
	printf(" 1<<3\t%8x\n", 1 << 3);
	printf("~1<<3\t%8x\n", 0xff & ~(1 << 3));
	printf("\t%8x\n", a);

	printf("÷√ 0");
	a = 0xff;
	a &= 0xff & ~(1 << 3);
	printf("\t%8x\n", a);


	return 0;
}

