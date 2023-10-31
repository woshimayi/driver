/*
 * @*************************************:
 * @FilePath: /user/C/string/signal_alarm.c
 * @version:
 * @Author: dof
 * @Date: 2023-10-13 16:06:48
 * @LastEditors: dof
 * @LastEditTime: 2023-10-13 17:11:56
 * @Descripttion:
 * @**************************************:
 */

#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

typedef struct
{
	int count;
	const char *message;
} TimerParams;

void timer_callback(int signum, siginfo_t *si, void *context)
{
	TimerParams *params = (TimerParams *)si->si_value.sival_ptr;
	printf("Timer triggered with count: %d and message: %s\n", params->count, params->message);
	params->count++;
}

int main()
{
	struct sigaction sa;
	sa.sa_flags = SA_SIGINFO;
	sa.sa_sigaction = timer_callback;
	sigemptyset(&sa.sa_mask);
	sigaction(SIGALRM, &sa, NULL);

	struct sigevent sev;
	sev.sigev_notify = SIGEV_SIGNAL;
	sev.sigev_signo = SIGALRM;

	TimerParams params;
	params.count = 0;
	params.message = "Hello, world!";

	sev.sigev_value.sival_ptr = &params;

	timer_t timerid;
	// 设置定时器，每秒触发一次
	if (timer_create(CLOCK_REALTIME, &sev, &timerid) == -1)
	{
		perror("timer_create");
		exit(EXIT_FAILURE);
	}

	struct itimerspec itime;
	itime.it_value.tv_sec = 1; // 初始延迟时间
	itime.it_value.tv_nsec = 0;
	itime.it_interval.tv_sec = 1; // 间隔时间
	itime.it_interval.tv_nsec = 0;

	// 启动定时器
	if (timer_settime(timerid, 0, &itime, NULL) == -1)
	{
		perror("timer_settime");
		exit(EXIT_FAILURE);
	}

	while (1)
	{
		sleep(1);
		// 执行其他任务
	}

	return 0;
}