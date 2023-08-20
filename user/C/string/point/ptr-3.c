#include <stdio.h>
#include <string.h>

void swap_2(int *a, int *b)
{
	int temp;
	// printf(">> formal addr a: %d, formal addr b: %d\n",&a,&b); //打印形参地址
	// printf(">> formal value a: %d, formal value b: %d\n",a,b); //打印形参的值
	// printf(">> formal addr value a: %d, formal addr value b: %d\n\n",*a,*b); //打印以形参值为地址的值
	temp = *a;
	*a = *b;
	*b = temp;
	// printf(">> formal addr a: %d, formal addr b: %d\n",&a,&b);
	// printf(">> formal value a: %d, formal value b: %d\n",a,b);
	// printf(">> formal addr value a: %d, formal addr value b: %d\n\n",*a,*b);
}

int main()
{
	int a = 1, b = 2;
	// printf(">> actual addr a: %d, actual addr b: %d\n", &a,&b);
	// printf(">> actual value a: %d, actual value b: %d\n\n",a,b);

	swap_2(&a, &b);
	printf("=====\n");
	printf(">> actual addr a: %p, actual addr b: %p\n", &a, &b);
	printf(">> actual value a: %d, actual value b: %d\n\n", a, b);
	return 0;
}
