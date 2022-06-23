#include <stdio.h>


#define T 100

void call_one()
{
	int count = T * 1000;
	while (count--);
}

void call_two()
{
	int count = T * 50;
	while (count--);
}

void call_three()
{
	int count = T * 20;
	while (count--);
}


int main(void)
{
	int time = 10;

	while (time--)
	{
		call_one();
		call_two();
		call_three();
	}

	return 0;
}
