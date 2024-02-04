/*
 * @*************************************: 
 * @FilePath: /user/C/string/trim.c
 * @version: 
 * @Author: dof
 * @Date: 2022-09-28 15:00:27
 * @LastEditors: dof
 * @LastEditTime: 2024-01-10 17:25:39
 * @Descripttion: 删除字符串两边的空格
 * @**************************************: 
 */



#include <string.h>
#include <stdio.h>
#include <ctype.h>
 
// delete the front whitespace
char *left_trim(char *str)
{
    char *beginp = str;
    char *tmp = str;
    while(isspace(*beginp)) beginp++;
    while((*tmp++ = *beginp++));
 
    return str;
}
 
// delete the back whitespace
char *right_trim(char *str)
{
    char *endp;
    size_t len = strlen(str);
 
    if(len == 0) return str;
 
    endp = str + strlen(str) - 1;
    while(isspace(*endp)) endp--;
    *(endp + 1) = '\0';
 
    return str;
}
 
/**
 * @brief 去除字符串两边的空格
 * 
 * @param str 
 * @return char* 
 */
char *trim(char *str)
{
    str = left_trim(str);
    str = right_trim(str);
 
    return str;
}
 

/**
 * @brief 去除字符串两边的空格
 * 
 * @param str 
 */
char * trim_space(char *str)
{
    char *p = str;
    char *q = str + strlen(str) - 1;

    while (*p == ' ')
    {
        p++;
    }

    while (*q == ' ')
    {
        q--;
    }

    if (p < q)
    {
        memmove(str, p, q - p + 1);
        str[q - p + 1] = '\0';
    }
    return str;
}

int main(void)
{
    char *src[] =
    {
        "   hello world   ",
        "    hello world",
        "hello world   ",
        "hello world",
        "",
        NULL
    };
    char result[1024];
     
    for(int index = 0; src[index] != NULL; index++)
    {
        strcpy(result, src[index]);
        // printf("[%s] -> [%s]\n", src[index], trim(result));          
        printf("[%s] -> [%s]\n", src[index], trim_space(result));          
    }
 
    return 0;
}