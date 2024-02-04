/*
 * @*************************************:
 * @FilePath: /user/C/string/trim_douoblePtr.c
 * @version:
 * @Author: dof
 * @Date: 2024-01-10 16:44:29
 * @LastEditors: dof
 * @LastEditTime: 2024-01-10 16:44:29
 * @Descripttion:
 * @**************************************:
 */

#include <stdio.h>
#include <string.h>

void trim_space(char *str)
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
}

int main()
{
    char str[] = "  hello world  ";

    trim_space(str);

    printf("%s\n", str);

    return 0;
}