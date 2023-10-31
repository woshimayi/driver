/*
 * @*************************************:
 * @FilePath: /user/C/string/signal_add_call_arg.c
 * @version:
 * @Author: dof
 * @Date: 2023-10-13 15:50:26
 * @LastEditors: dof
 * @LastEditTime: 2023-10-13 15:59:37
 * @Descripttion:  信号回调函数添加参数
 * @**************************************:
 */

#include <stdio.h>
#include <stdlib.h>
#include "signal.h"
#include "sys/types.h"
#include "unistd.h"
#include <sys/time.h>

void my_action(int signum, siginfo_t *info, void *myact)
{
	// 信号的响应操作
	printf("recive signal %d\n", signum);
	// 打印sigval的参数
	printf("int parmer %d\n", info->si_int);
}

void CreateSigAndBind(int Sig, void (*fun)(int signum, siginfo_t *info, void *myact))
{
	struct sigaction act;	  // 创建新的信号
	struct sigaction old_act; // 用与记录旧的信号，当然你也可以不用记录

	sigemptyset(&act.sa_mask);
	act.sa_flags = SIGALRM;
	act.sa_sigaction = fun; // 设置信号的响应操作

	if (sigaction(Sig, &act, &old_act) < 0)
	{ // 开始创建信号
		printf("install sigal error\n");
		return;
	}
}

int main(int argc, char **argv)
{
	// CreateSigAndBind(44, &my_action);

	struct itimerval timer;
	timer.it_value.tv_sec = 60;
	timer.it_value.tv_usec = 0;
	timer.it_interval.tv_sec = 0;
	timer.it_interval.tv_usec = 0;
	setitimer(ITIMER_REAL, &timer, NULL);
	// signal(SIGALRM, signal_handler);
	CreateSigAndBind(SIGALRM, &my_action);

	printf("pid = %d\n", getpid());



	// while (1)
	// { // 为了测试写的一个死循环
	// 	sleep(2);
	// 	printf("Now we wait for signal\n");
	// }
	return 1;
}
