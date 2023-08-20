#include <stdio.h>
#include <string.h>

void swap_1(int a, int b)
{
	int temp;
	temp = a;
	a = b;
	b = temp;
	printf(">> formal addr a: %d, formal addr b: %d\n", &a, &b);
	printf(">> formal value a: %d, formal value b: %d\n", a, b);
}

int main()
{
	int a = 1, b = 2;
	swap_1(a, b);
	printf(">> actual addr a: %d, actual addr b: %d\n", &a, &b);
	printf(">> actual value a: %d, actual value b: %d\n", a, b);
	return 0;
}
