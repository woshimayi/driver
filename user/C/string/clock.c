#include<stdio.h>
#include<stdlib.h>
#include<time.h>
void test()
{
	int a = 9894;
	while (a--)
	{
	}
}
int main()
{
	clock_t begin, end;
	begin = clock();
	test();
	end = clock();
	printf("%lf\n", (double)(end - begin) / CLOCKS_PER_SEC);
	return 0;
}
