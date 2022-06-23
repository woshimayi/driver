#include <stdio.h> 

#define A 12
#define B 13

#define _LINE(AA, BB) AA##BB
#define LINE(AA,BB)   _LINE(AA,BB)

int main()
{
	int n = LINE(A,B);
	printf("n = %d\n", n);
	return 0;
}
