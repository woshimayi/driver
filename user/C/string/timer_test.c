/*
 * @*************************************:
 * @FilePath: /user/C/string/timer_test.c
 * @version:
 * @Author: dof
 * @Date: 2023-12-28 14:27:51
 * @LastEditors: dof
 * @LastEditTime: 2023-12-28 19:12:59
 * @Descripttion: setitimer 定时器  注：这种定时器的的有效范围都仅在本进程内
 * @**************************************:
 */

#include <stdio.h>
#include <signal.h>
#include <sys/time.h>
#include <unistd.h>

void sig_alm_handler(int sig_num);

static int counter;

int main()
{
    printf("hello world.\n");

    // 拦截定时器信号。
    __sighandler_t *pre = signal(SIGALRM, sig_alm_handler);
    printf("pre-sighandler address:%p\n", pre); // pre应该是NULL.

    struct itimerval olditv;
    struct itimerval itv;
    itv.it_interval.tv_sec = 3; // 定时周期为1秒钟。
    itv.it_interval.tv_usec = 0;
    itv.it_value.tv_sec = 3; // 定时器启动以后将在3秒又500微秒以后正式开始计时。
    itv.it_value.tv_usec = 500;
    setitimer(ITIMER_REAL, &itv, &olditv);

    while (counter < 7)
    {
        usleep(500);
    }

    // try to cancle the timer.
    int counter2 = 0;
    while (counter2 < 7)
    {
        /*
            通过这个while可以监测到定时器是否成功被停止，
            若未停止，则仍旧可以在每秒看到定时信号回调函数中的打印。
            通过这个while可以确保定时器是真的被我们的设置而停掉的
            而非因为程序结束才强制停止的。
        */
        sleep(1);

        // try to stop the timer.
        setitimer(ITIMER_REAL, NULL, NULL);
        printf("try cancled!,counter2:%d\n", counter2);

        counter2++;
    }

    printf("\nBye.\n");

    return 0;
}

void sig_alm_handler(int sig_num)
{
    printf("%s, signal number:%d, counter:%d\n", __FUNCTION__, sig_num, counter);
    if (sig_num = SIGALRM)
    {
        counter++;
    }
}