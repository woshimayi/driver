/*
 * @*************************************:
 * @FilePath: /user/C/memory_valgrind_test.c
 * @version:
 * @Author: dof
 * @Date: 2024-08-02 15:00:32
 * @LastEditors: dof
 * @LastEditTime: 2024-08-06 16:17:30
 * @Descripttion:
 * @**************************************:
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


int main(int argc, char const *argv[])
{
    #if 1
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
#endif

    return 0;
}
