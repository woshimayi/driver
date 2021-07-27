/*
 * @*************************************:
 * @FilePath: /driver/user/C/time-1.c
 * @version:
 * @Author: dof
 * @Date: 2021-07-13 11:00:32
 * @LastEditors: dof
 * @LastEditTime: 2021-07-27 15:45:22
 * @Descripttion: 定时器 延时 循环
 * @**************************************:
 */

#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>

// 向标准错误输出信息，告诉用户时间到了
void prompt_info(int signo)
{
    printf("time is running out\n");
}

// 建立信号处理机制
void *init_sigaction(void *args)
{
    struct sigaction tact;
    /*信号到了要执行的任务处理函数为prompt_info*/
    tact.sa_handler = prompt_info;
    tact.sa_flags = 0;
    /*初始化信号集*/
    sigemptyset(&tact.sa_mask);
    /*建立信号处理机制*/
    sigaction(SIGALRM, &tact, NULL);

    struct itimerval value;
    /*设定执行任务的时间间隔为2秒0微秒*/
    value.it_value.tv_sec = 15;
    value.it_value.tv_usec = 0;
    /*设定初始时间计数也为2秒0微秒*/
    value.it_interval = value.it_value;
    /*设置计时器ITIMER_REAL*/
    setitimer(ITIMER_REAL, &value, NULL);
}

int main()
{
    struct itimerval curr_value;
    int ret;
    init_sigaction(NULL);

    while (1)
    {
        ret = getitimer(ITIMER_REAL, &curr_value); // 获取当前定时器状态
        if (0 == ret)
        {
            printf("%d %d\n", curr_value.it_value.tv_sec, curr_value.it_interval);
        }
        sleep(1);
    }

    return 0;
}
