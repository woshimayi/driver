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


int speed_test_result(const char * result)
{
    if (NULL == result)
    {
        return -1;
    }
    char str[1024] = {0};

    snprintf(str, sizeof(str), "%s", result);
    char *token, *rest = &str[0];
    int count = 0;
    int tmp = 0;

    while((token = strsep(&rest, "|")) != NULL) {
        if(*token != '\0') { // 忽略空字段
            printf("元素 %d: %s\n", ++count, token);
            if (tmp < atoi(token))
            {
                tmp = atoi(token);
            }
        }
    }
    
    printf("总共分割出 %d 个元素\n", count);
    return tmp;
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

    char *speed_str = "18526|0|11432|10936|0|7590|3670|13521|668|9286|0|10246|5855|8393|5929|10119|4142|6054|4169|6956";
    int restult = speed_test_result(speed_str);
    printf("result = %d", restult);

    return 0;
}
