/*
 * @*************************************:
 * @FilePath     : /user/C/tiemr_settime.c
 * @version      :
 * @Author       : dof
 * @Date         : 2024-08-21 11:09:11
 * @LastEditors  : dof
 * @LastEditTime : 2024-08-21 11:13:48
 * @Descripttion : 单线程定时器，适用于单例应用实例，循环定时器
 * @compile      :
 * @**************************************:
 */

#include <stdio.h>
#include <time.h>
#include <signal.h>

void timer_handler(int sig)
{
    printf("Timer expired!\n");
}

int main()
{
    timer_t timerid;
    struct sigevent sev;
    struct itimerspec its;

    // 创建定时器
    sev.sigev_notify = SIGEV_SIGNAL;
    sev.sigev_signo = SIGALRM;
    timer_create(CLOCK_REALTIME, &sev, &timerid);

    // 设置定时器
    its.it_value.tv_sec = 5; // 首次到期时间：5秒后
    its.it_value.tv_nsec = 0;
    its.it_interval.tv_sec = 2; // 间隔时间：2秒
    its.it_interval.tv_nsec = 0;
    timer_settime(timerid, 0, &its, NULL);

    // 注册信号处理函数
    signal(SIGALRM, timer_handler);

    // 等待信号
    while (1)
    {
        pause();
    }

    return 0;
}