#include <stdlib.h>
#include <stdio.h>
#include "call_function.h"
#include "vos_log.h"

#include <fcntl.h>
#include <getopt.h>
#include <dirent.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    //   double i = 34.45;
    //    char * s = "werwer";
    int k = 345;
    //   Caller2((void*)&i, Test2); //相当于调用Test2(30);
    //   Caller2((void*)s, Test3); //相当于调用Test2(30);
    Caller2((void *)&k, Test4); //相当于调用Test2(30);
    vos_print("test=%s,line=%d float=%f\n", "yunshouhu", 45, 1024.01);
    vos_error("test=%s,line=%d float=%f\n", "yunshouhu", 45, 1024.01);
    vos_notic("test=%s,line=%d float=%f\n", "yunshouhu", 45, 1024.01);
    vos_debug("test=%s,line=%d float=%f\n", "yunshouhu", 45, 1024.01);
    return 0;
}
