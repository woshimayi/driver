#include    "ourhdr.h"

int
main(void)
{
    printf("hello world from process ID %d\n", getpid());
    printf("hello world from parent process ID %d\n", getppid());
    printf("hello world from real user ID %d\n", getuid());
    printf("hello world from effective user ID %d\n", geteuid());
    printf("hello world from real group ID %d\n", getgid());
    printf("hello world from effective group ID %d\n", getegid());
	sleep(10);
    exit(0);
}
