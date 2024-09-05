/*
 * @*************************************:
 * @FilePath     : /user/C/random_test.c
 * @version      :
 * @Author       : dof
 * @Date         : 2024-09-04 14:18:53
 * @LastEditors  : dof
 * @LastEditTime : 2024-09-04 14:30:03
 * @Descripttion :
 * @compile      :
 * @**************************************:
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#undef PP
#define PP(fmt, args...) printf("\033[0;32;31m[mdm :%s(%d)] " fmt "\033[1;37m\r\n", __func__, __LINE__, ##args)

#ifndef UINT32_MAX
#define UINT32_MAX (4294967295U)
#endif

struct cpe_machine
{
    int cpe_retry_count;
    int cpe_retryCountdown;
};

void cwmpSetRetryWait(struct cpe_machine *m)
{
    unsigned int multiplier = 2000, i, random;
    unsigned long long min = 5, max = 10;

    multiplier /= 1000;
    max = min * multiplier;
    for (i = 1; i < m->cpe_retry_count && i < 3; i++)
    {
        if (min > UINT32_MAX || max > UINT32_MAX)
        {
            break;
        }
        min *= multiplier;
        max *= multiplier;
    }

    random = max - min > UINT32_MAX ? UINT32_MAX : max - min;
    random = random ? random : 30;
    m->cpe_retryCountdown = min + ((unsigned int)rand() % random);
    PP("cpe_retryCountdown  = %d", m->cpe_retryCountdown);
    // if (m->cpe_retry_count >= 3)
    {
        // m->cpe_retryCountdown = 3600;
    }
    PP("set wait time to %d (retry=%d)\n", m->cpe_retryCountdown, m->cpe_retry_count);
}

int main(int argc, char const *argv[])
{

    /* code */
    struct cpe_machine m;
    m.cpe_retry_count = 0;
    m.cpe_retryCountdown = 0;

    while (1)
    {
        m.cpe_retry_count++;
        cwmpSetRetryWait(&m);
        sleep(m.cpe_retryCountdown);
        m.cpe_retryCountdown--;
    }
    return 0;
}
