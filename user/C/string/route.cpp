#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main()
{
    char str[] =
        "224.0.0.239-239.255.255.255,219.155.50.0-219.155.51.255,10.254.176.0-10.254.191.254,221.13.223.128-221.13.223.158,202.99.114.0-202.99.114.126,103.3.99.224-103.3.99.238,60.28.127.12-60.28.127.14,60.28.127.16-60.28.127.22,10.254.192.0-10.254.255.255";
    char *p,  *startIp, *endIp = NULL;
    char buf[20][64] = {0};
    printf("str = %s\n%d\n", str, strlen(str) + 1);
    int i, k = 0;

    p = strtok(str, ",");
    while (p != NULL)
    {
        strncpy(buf[i], p, strlen(p) + 1);
        printf("%d = %s\n", i, p);
        p = strtok(NULL, ",");
        i++;
    }
    for (k = 0; k < i; k++)
    {
        //		printf("buf[%d] = %s\n", k, buf[k]);
        startIp = strtok(buf[k], "-");
        if (startIp != NULL)
        {
            endIp = strtok(NULL, "-");
            printf("startIp = %12s			 endIp = %12s\n", startIp,  endIp);
        }
    }


    return 0;
}
