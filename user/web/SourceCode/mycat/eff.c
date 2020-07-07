#include    "ourhdr.h"
#include	<math.h>

int
main(int argc, char *argv[])
{
    int     n, buf_size;
    char    *buf;

	if(argc != 2)
	{
		err_quit("must have a number argument.");
	}
	
	buf_size = atoi(argv[1]);
	buf = malloc(buf_size);

   	while ( (n = read(STDIN_FILENO, buf, buf_size)) > 0)
       	if (write(STDERR_FILENO, buf, n) != n)
           	err_sys("write error");

	if (n < 0)
		err_sys("read error");

	free(buf);

    exit(0);
}
