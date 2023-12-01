/*
 * @*************************************: 
 * @FilePath: /user/C/string/point/ptr_test.c
 * @version: 
 * @Author: dof
 * @Date: 2023-10-31 14:04:56
 * @LastEditors: dof
 * @LastEditTime: 2023-11-07 14:24:48
 * @Descripttion: 
 * @**************************************: 
 */
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

    printf("%d\n", strcasecmp("zhengsen.dof.com", "zHengsen.Dof.com"));

    return 0;
}