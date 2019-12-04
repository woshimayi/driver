#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main()
{
	FILE * fd = NULL;
	char  buf[128] = {0};
	
	fd = fopen("./soft_feature.cfg", "r");
	if (fd == NULL)
	{
		printf("open error");
		return 1;
	}
	
	while (NULL != fgets(buf, sizeof(buf), fd))
	{
		printf("%s", buf);
	}

	return 0;
}


