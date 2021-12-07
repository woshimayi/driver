#include<stdio.h>
#include<string.h>
#include<stdlib.h>

int main()
{
    char *num = "qwertyuiop";
    char *num1 = "345";
    num = strstr(num, num1);
    printf("num = %s num1 = %s\n", num, num1);
    return 0;
}

