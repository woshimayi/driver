#include <stdio.h>
#include <stdlib.h>
#include <string.h>


double db = 0;

void ping(int str, double str1)
{
    FILE *fp = NULL;
    char cmd[48] = {0};
    char buf[128] = {0};
    int totalmm = 0;

    double totalfree = 0;

    double L = 0;

    char p[64];

    //    4 packets transmitted, 4 received, 0% packet loss, time 3003ms
    //    rtt min/avg/max/mdev = 6.927/8.696/9.942/1.107 ms
    sscanf("received, 43% packet loss, time 6008ms", "%*s%d%*s", &totalmm);
    printf("mm = %d\n", totalmm);
    sscanf("mdev = 7.694/8.347/9.092/0.565 ms", "%*[^/]/%[^/]", buf);
    L = atof(buf);
    printf("%.3f\n", L);
    str1 = L;
    str = totalmm;
}

int main()
{
    int str = 0;
    double str1 = 0;

    ping(str, str1);
    printf("main str = %d str1 = %f\n", str, str1);
    return 0;
}
