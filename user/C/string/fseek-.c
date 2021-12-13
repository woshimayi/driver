#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define PP printf("date = %s", date);

int main(int atgc, char *argv[])
{
	FILE *fd = NULL;
	char date[63] = {0};

	fd = fopen("one.mov", "rb");
	if (NULL == fd)
	{
		printf("fail open file");
	}
	fseek(fd, -10, 2);
	fread(date, 1, 63, fd);
	PP
	fclose(fd);

	return  0;
}

