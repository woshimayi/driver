#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define STR(s) #s
#define CONS(a,b) int(a##e##b)




int main()
{
	printf(STR(sdfsd));
	printf("\n%d", CONS(2,3));
	return 0;
}


