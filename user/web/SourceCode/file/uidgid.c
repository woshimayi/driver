#include    "ourhdr.h"

int
main(void)
{
    printf(" uid = %d,  gid = %d\n", getuid(), getgid());
    printf("euid = %d, egid = %d\n", geteuid(), getegid());
    exit(0);
}
