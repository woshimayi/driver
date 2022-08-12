/*
 * @*************************************: 
 * @FilePath: /user/C/ptr_fun_strcpy.c
 * @version: 
 * @Author: dof
 * @Date: 2021-09-20 18:22:18
 * @LastEditors: dof
 * @LastEditTime: 2021-09-21 16:11:22
 * @Descripttion:  指针函数 malloc
 * @**************************************: 
 */

#include <stdio.h>
#include <string.h>

char *str_cat(char *dest, char *src)
{
    printf("%s %s\n", dest, src);

    strcpy(dest, src);
    return dest;
}

char *str_cat_1(char **dest, char *src)
{
    printf("%s\n", src);

    strcpy(*dest, src);
    printf("src %p %s\n *dest  %p %s\n **dest %p %s\n", src, src, *dest, *dest, dest, dest);
    puts(*dest);
    return *dest;
}

int main(int argc, char const *argv[])
{
    char s1[20] = "Chinese";
    char s2[20] = "Dream";
    char *s3 = NULL;
    printf("main 1 %p\n", s3);
    char *p = str_cat_1(&s3, s2);
    printf("main 2 %p %p %p\n", p, s3, &s3);
    puts(p);
    puts(s3);
    return 0;
}
