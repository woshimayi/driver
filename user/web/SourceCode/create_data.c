#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int
main(void)
{
	int	i;
	char s[50];

	for(i = 0; i < 49; i++)
	{
		s[i] = i;
	}
	s[49]= '\0';
	for(i = 0; i < 30000; i++)
	{
		write(STDOUT_FILENO, s, 50);
	}
	exit(EXIT_SUCCESS);
}

