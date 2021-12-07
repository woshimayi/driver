#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int opensyslog_tw(char *str)
{
    FILE *fd = NULL;
    fd = fopen("syslog_login.txt", "w+");

    time_t timep;
    time(&timep);
    fprintf(fd, "%s: %s", ctime(&timep), str);

    fclose(fd);
}

int main()
{
    opensyslog_tw("asdas");
    return 0;
}


