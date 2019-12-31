#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "build/HelloConfigure.h"

#ifdef USE_MYFUNC
#include "world.h"
#endif


int main(int argc, char *argv[])
{
    if (argc >= 2)
    {
        fprintf(stdout, "%s Version %d.%d\n", argv[0], Hello_VERSION_MAJOR, Hello_VERSION_MINOR);
        fprintf(stdout, "Usage: %s number\n", argv[0]);
        return 1;
    }

    printf("Hello ");
#ifdef USE_MYFUNC
    func();
#else 
    printf("not func function");
#endif

    return 0;
}