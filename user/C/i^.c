#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main()
{
    int i = 2;
    int k = 1;
    while (1)
    {
        scanf("%d", &k);
        i = 2;
        printf("i = %d\n", i << k);
    }
    return 0;
}


