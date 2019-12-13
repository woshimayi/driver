#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define SF_GetFeature(id)  _##id##_


int main()
{
    printf("%s\n", SF_GetFeature(3));

    return 0;
}


