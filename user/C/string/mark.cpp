#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main()
{
	int mark = 1;
	scanf("%d", &mark);
	printf("0x%x\n", mark);
	mark = mark & 0xf;
	printf("0x%x\n", mark);

	if (1)
	{
		mark = (mark << 0x4);
		printf("0x%x\n", mark);
	}
	else
	{
		mark >> 0x4;
		printf("0x%x\n", mark);
	}
	//	printf("0x%x\n", mark);
	return 0;
}



//mark& 0xf
