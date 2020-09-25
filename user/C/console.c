/*
	Name:
	Copyright:
	Author:
	Date: 25/09/20 16:47
	Description: �ն˳��� ָ��λ��ˢ��
*/

/*
���¼򵥵�˵����
              \e[ �� \033[ �� CSI������������Ļ�ġ�
              \e[K ��ʾ�ӹ�굱ǰλ����ɾ���� EOL ����β��
              \e[NX ��ʾ�������X�����ƶ�N��X = A(��) / B(��) / C(��) / D(��)��\e[1A ���ǰѹ�������ƶ�1��

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

        printf("\033[%dA", j); //�Ȼص���һ��
        printf("\033[K");  //�������
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
        printf("\033[%dA", j); //�Ȼص���һ��
        printf("\033[K");  //�������
    }
    return 0;
}
