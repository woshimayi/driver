#include <stdio.h>
#include <stdlib.h>
#include <string.h>


void xg_macStringToHex(char *macstring, unsigned char *machex)
{
    char *res = NULL;
    char *savePtr = NULL;
    while (NULL != (res = strtok_r(macstring, ":", &savePtr)))
    {
        printf(" %4s\n", res);
        *machex++ = (char)strtol(res, NULL, 16);
        macstring = NULL;
        if (NULL == machex)
        {
            break;
        }
    }
}



int main()
{
    char str1[] = "74:b9:eb:cb:0d:b2";
    char str2[] = "74:b9:eb:cb:0d:b7";
    printf("%s\n %s\n", str1, str2);
    xg_macStringToHex(str1, str2);
    printf("%x\n %x\n", str1, str2);

    return 0;
}
