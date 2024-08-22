/*
 * @*************************************:
 * @FilePath: /user/C/memory_valgrind_test.c
 * @version:
 * @Author: dof
 * @Date: 2024-08-02 15:00:32
 * @LastEditors: dof
 * @LastEditTime: 2024-08-07 10:35:18
 * @Descripttion:
 * @**************************************:
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifndef PP
#define PP(fmt,args...) printf("\033[0;33;34m[zzzzz :%s(%d)] " fmt "\033[1;37m\r\n", __func__, __LINE__, ##args )
#endif



int main(int argc, char const *argv[])
{
    PP("sss");
    #if 0
    char *vec[100];
    sleep(1);
    for (int i = 0; i < 100; i++)
    {
        vec[i] = (char *)malloc(100);
        strcpy(vec[i], "Hello, world!");
        printf("str: vec[%d] = %s\n", i, vec[i]);
    }
    sleep(1);
    for (int i = 0; i < 100; i++)
    {
        printf("str: vec[%d] = %s\n", i, vec[i]);
        if (20 == i)
        {
            break;
        }
        free(vec[i]);
    }
    #elif 0
    char str[18];
    printf("str = %s\n", &str[1]);
    printf("str = %s\n", &str[11]);
    printf("str = %s\n", &str[20]);
    #elif 0
    char *str = (char *)malloc(18);
    strcpy(str, "Hello, world!");
    printf("str = %s\n", str);
    printf("str = %s\n", str+25);
    if (1)
    {
        free(str);
        return 0;
    }
    free(str);
    str = NULL;

#endif

    return 0;
}
