/*
 * @*************************************: 
 * @FilePath: /user/C/string/trim.c
 * @version: 
 * @Author: dof
 * @Date: 2022-09-28 15:00:27
 * @LastEditors: dof
 * @LastEditTime: 2022-09-29 09:44:04
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
 
char *trim(char *str)
{
    str = left_trim(str);
    str = right_trim(str);
 
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
        printf("[%s] -> [%s]\n", src[index], trim(result));          
    }
 
    return 0;
}