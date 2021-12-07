/*
 * @*************************************: 
 * @FilePath: /user/C/string/6838_admin_passwd.c
 * @version: 
 * @Author: dof
 * @Date: 2021-10-20 19:33:39
 * @LastEditors: dof
 * @LastEditTime: 2021-12-07 13:58:29
 * @Descripttion: 加密
 * @**************************************: 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

int i64c(int i)
{
    if (i <= 0)
        return ('.');
    if (i == 1)
        return ('/');
    if (i >= 2 && i < 12)
        return ('0' - 2 + i);
    if (i >= 12 && i < 38)
        return ('A' - 12 + i);
    if (i >= 38 && i < 63)
        return ('a' - 38 + i);
    return ('z');
}


char *util_cryptMakeSalt(void)
{
    time_t now;
    static unsigned long x;
    static char result[3];

    time(&now);
    x += now + getpid() + clock();
    result[0] = i64c(((x >> 18) ^ (x >> 6)) & 077);
    result[1] = i64c(((x >> 12) ^ x) & 077);
    result[2] = '\0';

    return result;
}



int main()
{
    char *str = crypt("34BF90268626", NULL);

    printf("%s\n", str);

    return 0;
}


