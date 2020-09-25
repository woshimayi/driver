/*
	Name:
	Copyright:
	Author:
	Date: 25/09/20 16:47
	Description: 终端程序 指定位置刷新
*/

/*
以下简单的说明：
              \e[ 或 \033[ 是 CSI，用来操作屏幕的。
              \e[K 表示从光标当前位置起删除到 EOL （行尾）
              \e[NX 表示将光标往X方向移动N，X = A(上) / B(下) / C(左) / D(右)，\e[1A 就是把光标向上移动1行

              A UP /B down /C left /D right
*/


#include <stdio.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>


void *setTimer(int usec)
{
    struct timeval temp;

    temp.tv_sec = 0;
    temp.tv_usec = usec;
    //	printf("ssssssssss\n");
    select(0, NULL, NULL, NULL, &temp);
    //    printf("timer\n");
}

int j = 0;
#define log(fmt, ...)    log_log(fmt, ##__VA_ARGS__)

int log_log(const char *fmt, ...)
{
    int ret = 0;
    int len = 0;
    char buf[256] = {0};
    char *cmd = NULL;
    char *allocBuf = NULL;
    va_list paraList;

    va_start(paraList, fmt);
    len = vsnprintf(buf, sizeof(buf), fmt, paraList);
    va_end(paraList);

    if (len < sizeof(buf))
    {
        cmd = buf;
    }
    else
    {
        len++;
        allocBuf = (char *)malloc(len);
        va_start(paraList, fmt);
        vsnprintf(allocBuf, len, fmt, paraList);
        va_end(paraList);

        cmd = allocBuf;
    }
    j++;
    printf("%s", cmd);
//    fflush(stdout);
    setTimer(100);

    free(allocBuf);
    return ret;
}


int main(void)
{
    int i, l = 0;
    int k = 115;
    while (1)
    {
        j = 0;
        for (i = 1; i <= k / 2; i++)
        {
            for (l = (k + 1) / 2 - i; l > 0; l--)
            {
                log(" ");
                j--;
            }
            for (l = 1; l <= 2 * i - 1; l++)
            {
                log("*");
                j--;
            }
            for (l = (k + 1) / 2 - i; l > 0; l--)
            {
                log(" ");
                j--;
            }
            log("\n");
        }

        printf("\033[%dA", j); //先回到上一行
        printf("\033[K");  //清除该行
        j = 0;
        for (i = k / 2; i > 0; i--)
        {
            for (l = (k + 1) / 2 - i; l > 0; l--)
            {
                log(" ");
                j--;
            }
            for (l = 1; l <= 2 * i - 1; l++)
            {
                log("*");
                j--;
            }
            for (l = (k + 1) / 2 - i; l > 0; l--)
            {
                log(" ");
                j--;
            }
            log("\n");
        }
        printf("\033[%dA", j); //先回到上一行
        printf("\033[K");  //清除该行
    }
    return 0;
}
