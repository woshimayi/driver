/*
 * @*************************************:
 * @FilePath: /user/C/string/strsep_test.c
 * @version:
 * @Author: dof
 * @Date: 2022-11-30 17:24:31
 * @LastEditors: dof
 * @LastEditTime: 2022-12-29 16:32:15
 * @Descripttion: 字符切割函数测试  使用strsep 替代 strtok
 * @**************************************:
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


void dof_cut(char *input, char *out[])
{
    char *buf;
    char *token;
    buf = input;
    int i = 0;
    while ((token = strsep(&buf, ",")) != NULL)
    {
        printf("%s\n", token);
        // out[i++] = token;
        strncpy(out[i++], token, 128);
    }
}

int main(void)
{
    // char str[] = "root:x::0:root:/root:/bin/bash:";
    // char str[] = "www.baidu.com,www.taobao.com,www.jd.com";
    char *str= "www.baidu.com";
    char out[32][128] = {0};
    int i = 0;
    char *buf;
    char *token;
    buf = str;
    while ((token = strsep(&buf, ",")) != NULL)
    {
        printf("111 %s\n", token);
        strncpy(out[i++],token, 128);
    }
    printf("str = %p buf = %p", str, buf);
    // dof_cut(str, out);

#if 0
    char *test_str = "http://192.168.1.1/login.html";
    char *host = "192.168.1.1";
    char *hostref = "http://192.168.1.1/";
    char *path = "/login.html";
    // printf("%s\n", strstr(test_str, host));
    // printf("%s\n", strstr(host, test_str));
    printf("path = %p %s\n", path, path);
    path = "ssssdddddddddddddddddd";
    printf("path = %p %s\n", path, path);
#endif

    return 0;
}
