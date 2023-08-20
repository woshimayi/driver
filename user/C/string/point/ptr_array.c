/*
 * @*************************************: 
 * @FilePath: /user/C/ptr_array.c
 * @version: 
 * @Author: dof
 * @Date: 2021-09-23 21:19:05
 * @LastEditors: dof
 * @LastEditTime: 2021-09-26 22:08:49
 * @Descripttion: 
 * @**************************************: 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char const *argv[])
{
    char *s1 = "aaa";
    char *s2 = "bbb";
    char s4[64] = {0};
    // char *s3 = s1;
    // s3 = s2;

    // puts(s3 + 1);
    // puts(s3 + 2);
    // strcpy(s3, "sss");
    int a = 2;
    // int *s3 = (int *)malloc(100);
    char *s3 = (char *)malloc(100);
    // s3 = &a;
    // *s3 = a;
    // printf("%p %p %p\n", &s4[0] + 1, &s4[0] + 2, &s4 + 1);
    // printf("%s %s %s\n", s4 + 2, s4 + 4, s4);
    // s3 = s4;
    strcpy(s3, "ssssssssssssss");
    strcpy((s3 + 4), s2);
    // s3[4] = "dddd";
    printf("%p %s %s\n", s3 + 1, (s3 + 1), s3);

    char *p = "asd**fgh***jklo***";
    int i = 0, j = strlen(p) - 1;
    while (*p != '\0')
    {
        if (!strchr("*", *p))
        {
            s4[i++] = *p;
        }
        else
        {
            s4[j--] = '*';
        }
        p++;
    }

    if (strchr(s4, '*'))
    {
        puts(strchr(s4, '*'));
    }

    // i = 0, j = strlen(p) - 1;
    // memset(s4, '\0', sizeof(s4));
    // while (*p != '\0')
    // {
    //     if (!strchr("*", *p))
    //     {
    //         s4[j--] = *p;
    //     }
    //     else
    //     {
    //         s4[i++] = '*';
    //     }
    //     p++;
    // }

    puts(s4);

    printf("%c\n", 'a' + '6' + '4');
    // free(s3);
    return 0;
}
