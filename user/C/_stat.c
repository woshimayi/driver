#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/stat.h>
#include <unistd.h>


main()
{
    char cmd[128] = {0};
    int count = 0;

    struct stat buf;

    while (buf.st_size < 1000000)
    {
        stat("./123", &buf);

    }
    printf("/etc/passwd file size = %d\n", buf.st_size);



    FILE *fd = NULL;
    fd =  fopen("./123", "r");
    while (fread((void *)cmd, 1, 25, fd))
    {
        printf("%s", cmd);
    }


}

