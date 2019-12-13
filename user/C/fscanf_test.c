#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main()
{

    int i;
    char s[128] = {0};
    printf("please input intereger:");
    if (fscanf(stdin, "%s", s))
        printf("%s\n", s);
    else
    {
        fprintf(stderr, "sdfsdfsdfsdfsd\n");
        exit(1);
    }
    return 0;
}


