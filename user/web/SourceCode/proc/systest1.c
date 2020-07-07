#include    <sys/types.h>
#include    <sys/wait.h>
#include    "ourhdr.h"

int
main(void)
{
    int        status;

    if ( (status = system("date")) < 0)
        err_sys("system() error");
    pr_exit(status);
	write(STDOUT_FILENO, "========================================\n",41);

    if ( (status = system("nosuchcommand")) < 0)
        err_sys("system() error");
    pr_exit(status);
	write(STDOUT_FILENO, "========================================\n",41);

    if ( (status = system("who; exit 44")) < 0)
        err_sys("system() error");
    pr_exit(status);
	write(STDOUT_FILENO, "========================================\n",41);

    exit(0);
}
