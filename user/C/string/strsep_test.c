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
#include <math.h>
#include <unistd.h>

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

int speed_test_result(const char *result)
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

    while ((token = strsep(&rest, "|")) != NULL)
    {
        if (*token != '\0')
        { // 忽略空字段
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

int count_left_shift(unsigned int num)
{
    int count = 0;
    if (num == 0)
    {
        return -1; // 0 不是 1 左移得到的
    }
    while (num != 1)
    {
        if (num & 1)
        {
            return -1; // 如果不是 2 的幂次方，返回 -1
        }
        num >>= 1;
        count++;
    }
    return count;
}

int main(void)
{
#if 0
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
    printf("str = %p buf = %p\n", str, buf);
    // dof_cut(str, out);

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
    // char *speed_str = "583831|34970|373222|205978|205809|35830|329649|55956|300576|207322|200686|0|231794|291344|133592|0|322913|290593|205844|170640";
    // int restult = speed_test_result(speed_str);
    // printf("result = %d", restult);

    // #define CWMP_WRITE 0x01
    // #define CWMP_READ 0x02
    // #define CWMP_LNKLIST 0x04
    // #define CWMP_DENY_ACT 0x08
    // #define CWMP_FORCE_ACT 0x10
    // #define CWMP_ISPASSWORD 0x20
    // #define CWMP_ISVOIP 0x40
    // #define CWMP_ISWLAN 0x80
    // #define CWMP_HIDDEN 0x100

    //     int i = CWMP_WRITE | CWMP_READ | CWMP_ISWLAN;
    //     printf("%x %x\n", i, ~CWMP_ISWLAN);
    //     i &= ~CWMP_ISWLAN;
    //     i |= CWMP_HIDDEN;
    //     printf("%x\n", i);

    //     double f = 456.13212;
    //     printf("%0.2f\n", f);

    // int num = 0x4; // 0x8 = 8 (二进制 1000)
    // int shift = count_left_shift(num);
    // if (shift != -1)
    // {
    //     printf("0x%x 是 1 向左移动 %d 位得到的\n", num, shift);
    // }
    // else
    // {
    //     printf("0x%x 不是 1 左移得到的\n", num);
    // }
    // printf("%d\n", __builtin_ctz(0x08));

    // printf("%d\n", access("/tmp/123", F_OK));

    // long long totBytes = 2253776170;
    // long long max_totBytes = 2147483648;
    // long long TestFileLenth = 2147483648;
    // int real_dur = 20542452;

    
    // double maxrate = (totBytes * 8) / (real_dur / 1000.0) / 1024;
    
    // totBytes = (totBytes > max_totBytes) ? max_totBytes : totBytes;
    
    // int dur = 0;
    // dur = (totBytes * 8) /(maxrate * 1024) * 1000;
    // printf("maxrate = %f dur = %d, sec = %d usec = %d\n", maxrate, dur, dur/1000000, dur%1000000);


    #define FILE_TMP "123"
    system("ls -l " FILE_TMP);

    return 0;
}
