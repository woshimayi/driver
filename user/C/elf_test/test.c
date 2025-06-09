#include <stdio.h>
#include <stdlib.h>
#include <string.h>




int main(int argc, char const *argv[])
{
    char *str = (char *)malloc(128);
    snprintf(str, 128, "dddddd");
    printf("str = %s = %d\n", str, strlen(str));
    free(str);
    str = NULL;
    printf("str = %s = %d\n", str, strlen(str));
    return 0;
}
