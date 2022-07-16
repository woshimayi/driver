#include    "ourhdr.h"

int
main(int argc, char *argv[])
{
    int            i;
    char        **ptr;
    extern char    **environ;

    for (i = 0; i < argc; i++)        /* echo all command-line args */
        printf("argv[%d]: %s\n", i, argv[i]);

	printf("\n\n==================================\n\n");
    for (ptr = environ; *ptr != 0; ptr++)    /* and all env strings */
        printf("%s\n", *ptr);
	printf("\n\n==================================\n\n");

    exit(0);
}
