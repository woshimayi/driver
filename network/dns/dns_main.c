



#include <stdio.h>
#include <string.h>
#include <stdio.h>
#include "parse_dns.h"


int main(int argc, char const *argv[])
{
    char buf[128] = {0};
    ParaseIpv4Domain("eth0", "www.jd.com", "223.5.5.5", buf);
    printf("buf = %s\n", buf);
    return 0;
}

