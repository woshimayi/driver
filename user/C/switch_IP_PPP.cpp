#include<stdio.h>
#include<string.h>
#include<stdlib.h>

int main()
{
    int num = 0;
    int num1 = 0;
    bool flag = false;
    bool flagP = false;

    if (flag || flagP)
    {

    }

    switch (num || num1)
    {
        case 1:
            printf("DHCP\n");
            break;
        case 0:
            printf("STATIC\n");
            break;
        case 2:
            printf("PPPOE\n");
            break;
    }

    return 0;
}

