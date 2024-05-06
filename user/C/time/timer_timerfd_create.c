/*
 * @*************************************: 
 * @FilePath: /user/C/time/timer_timerfd_create.c
 * @version: 
 * @Author: dof
 * @Date: 2024-04-09 09:32:04
 * @LastEditors: dof
 * @LastEditTime: 2024-04-09 10:43:06
 * @Descripttion: 
 * @**************************************: 
 */

/*
struct itimerval 是 Linux 内核中定义的一个结构体，用于表示定时器值。 它包含两个成员：

it_interval: 表示定时器的间隔时间。
it_value: 表示定时器的初始值。
这两个成员都是 timeval 结构体类型。 timeval 结构体包含两个成员：

tv_sec: 表示秒数。
tv_usec: 表示微秒数。
以下是 struct itimerval 参数的详细说明：

it_interval.tv_sec: 表示定时器每隔多少秒触发一次。
it_interval.tv_usec: 表示定时器每隔多少微秒触发一次。
it_value.tv_sec: 表示定时器首次触发的时间间隔，以秒为单位。
it_value.tv_usec: 表示定时器首次触发的时间间隔，以微秒为单位。
您可以使用 setitimer() 函数设置定时器。 setitimer() 函数的第一个参数是定时器类型，可以取以下值：

ITIMER_REAL: 计时器从调用 setitimer() 函数时开始计时。
ITIMER_VIRTUAL: 计时器只有在进程执行用户代码时才会计时。
ITIMER_PROF: 计时器只有在进程执行用户代码或内核代码时才会计时。
setitimer() 函数的第二个参数是要设置的定时器值。 第三个参数是旧的定时器值，可以设置为 NULL。

当定时器超时时，会向进程发送一个信号。 信号的编号可以通过以下公式计算：

signal number = timer type + 2
例如，如果定时器类型为 ITIMER_REAL，那么超时时会向进程发送一个 SIGALRM 信号。
*/

#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <signal.h>

void handler(int sig, siginfo_t *info, void *ucontext) {
    // 处理超时事件
    // 重新设置定时器
    printf("zzzzz \n");
    struct itimerval value;
    value.it_interval.tv_sec = 1;
    value.it_interval.tv_usec = 0;
    value.it_value.tv_sec = 1;
    value.it_value.tv_usec = 0;
    setitimer(ITIMER_REAL, &value, NULL);
}

int main() {
    
    // 设置定时器
    struct itimerval value;
    value.it_interval.tv_sec = 1;
    value.it_interval.tv_usec = 0;
    value.it_value.tv_sec = 1;
    value.it_value.tv_usec = 0;
    setitimer(ITIMER_REAL, &value, NULL);

    // 注册信号处理函数
    struct sigaction sa;
    sa.sa_sigaction = handler;
    sa.sa_flags = SA_SIGINFO;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGALRM, &sa, NULL);

    // 循环等待超时
    while (1) {
        // ...
        // printf("zzzzz while \n");
    }

    return 0;
}

