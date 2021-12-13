#include <stdio.h>
#include <string.h>
#include <stdlib.h>


typedef struct asd
{
	int a;
	int b;
	double c;
} _asd;


int main()
{
	_asd *A;
	_asd *B;
	_asd *C;

	A->a = 8;
	A->b = 4;
	A->c =  3.5;

	B->a = 8;
	B->b = 4;
	B->c =  3.5;

	C->a = 8;
	C->b = 5;
	C->c =  4.5;


	printf("%p\n%p\n%p\n", &A, &B, &C);
	if (0 == memcmp((void *)A, (void *)B, sizeof(_asd)))
	{
		printf("success\n");
	}
	else
	{
		printf("fail\n");
	}

	// printf("", A);

	// while ()
	// {

	// }





	return 0;
}


