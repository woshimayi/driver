#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void fun(char **p)
{
    char str[128] = "1234567890";
    printf("fun 1 %p \n", str);
    *p = (char *)malloc(100);
    // *p = str;
    strcpy(*p, str);
    printf("fun 2 %p %s\n", *p, *p);
}

void freeStr(char **p)
{
    free(*p);
}

int main(int argc, char *argv[])
{
    char *str1;
    printf("%p %s %p\n", &str1, str1, &str1);
    fun(&str1);
    printf("%p %s %p\n", &str1, str1, &str1);
    // freeStr(&str1);
    free(str1);
    return 0;
}