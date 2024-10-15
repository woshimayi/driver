/*
 * @*************************************:
 * @FilePath     : /user/C/tiemr_settime.c
 * @version      :
 * @Author       : dof
 * @Date         : 2024-08-21 11:09:11
 * @LastEditors  : dof
 * @LastEditTime : 2024-10-15 10:43:12
 * @Descripttion : 单线程定时器，适用于单例应用实例，循环定时器
 * @compile      :
 * @**************************************:
 */

#include <stdio.h>
#include <time.h>
#include <signal.h>

#define DYNC_CHANGE 1        // 0:初始化时调整，1：动态调整，在回调函数中调整下次执行间隔

#if DYNC_CHANGE
    timer_t timerid;
#endif

void timer_handler(int sig)
{
    printf("Timer expired!\n");
    struct itimerspec its;

#if DYNC_CHANGE
    static int timer = 1;
    timer++;
    printf("next timer %d expired!\n", 1 << timer);

    // 设置定时器
    its.it_value.tv_sec = 1<<timer; // 首次到期时间：5秒后
    its.it_value.tv_nsec = 50000;
    its.it_interval.tv_sec = 0; // 间隔时间：2秒, 如果为0 则为一次
    its.it_interval.tv_nsec = 0;
    timer_settime(timerid, 0, &its, NULL);
#endif
    // exit(0);
}

void dof_time_create(int time)
{
    struct sigevent sev;
    struct itimerspec its;
    #if !DYNC_CHANGE
    timer_t timerid;
    #endif

    // 创建定时器
    sev.sigev_notify = SIGEV_SIGNAL;
    sev.sigev_signo = SIGALRM;
    timer_create(CLOCK_REALTIME, &sev, &timerid);

    // 设置定时器
    its.it_value.tv_sec = time; // 首次到期时间：5秒后
    its.it_value.tv_nsec = 50000;
    its.it_interval.tv_sec = 0; // 间隔时间：2秒, 如果为0 则为一次
    its.it_interval.tv_nsec = 0;
    timer_settime(timerid, 0, &its, NULL);

    // 注册信号处理函数
    signal(SIGALRM, timer_handler);
}

int main()
{
    dof_time_create(10);

    int i = 0;
    // 等待信号
    while (1)
    {
        // pause();
        sleep(1);
        if (i > 10 && i < 15)
        {
            // signal(SIGALRM, SIG_IGN);       // 忽略信号
        }
        else if (i > 15)
        {
            // signal(SIGALRM, timer_handler); // 重新设置SIGALRM信号，回调生效
        }
        i++;
        // printf("zzzzz while i = %d\n", i);
    }

    return 0;
}