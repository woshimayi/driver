#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "build/Configure.h"

#ifdef SRC_FUNC
#include "head.h"
#endif


int main(int argc, char *argv[])
{
    int ret = -1;
    char *url = "http://www.baidu.com";
    char host[128] = {0};
    int port = 80;
    char buf[32] = {0};

    if (argc >= 2)
    {
        fprintf(stdout, "%s Version %d.%d\n", argv[0], VERSION_MAJOR, VERSION_MINOR);
        fprintf(stdout, "Usage: %s number\n", argv[0]);
        return 1;
    }

    www_ParseUrl(url, NULL, host, &port, NULL);
    port = port?port:80;
    sprintf(buf, "%d", port);
    printf("host= %s\nport=%s\nifname=%s\n", host, buf, argv[1]);

    ret = ifname_test(host, buf, argv[1]);
    printf("ret = %d\n", ret);

    return 0;
}