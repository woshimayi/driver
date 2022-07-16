#include	<unistd.h>
#include	<fcntl.h>
#include    "ourhdr.h"

#define    BUFFSIZE    8192

int
main(void)
{
    int     n;
    char    buf[BUFFSIZE];

	set_fl(STDERR_FILENO, O_SYNC);

    while ( (n = read(STDIN_FILENO, buf, BUFFSIZE)) > 0)
        if (write(STDERR_FILENO, buf, n) != n)
            err_sys("write error");

    if (n < 0)
        err_sys("read error");

    exit(0);
}
