#include <stdio.h>
#include <time.h>

int main(void)
{
	time_t result;

	result = time(NULL);
	printf("%s%u\n", asctime(localtime(&result)), result);
	return(0);
}
