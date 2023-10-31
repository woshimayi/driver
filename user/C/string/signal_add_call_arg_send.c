/*
 * @*************************************:
 * @FilePath: /user/C/string/signal_add_call_arg_send.c
 * @version:
 * @Author: dof
 * @Date: 2023-10-13 15:51:54
 * @LastEditors: dof
 * @LastEditTime: 2023-10-13 15:51:54
 * @Descripttion:  发送信号
 * @**************************************:
 */

#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MYSIG 13

int main(int argc, char **argv)
{
	pid_t pid = (pid_t)atoi(argv[1]);

	// 发送的信号值
	int signo = MYSIG;

	// 附带在信号上的参数
	union sigval mysigval;
	mysigval.sival_int = 8;
	// sigval支持也 void *sival_ptr;

	// 发送信号
	if (sigqueue(pid, signo, mysigval) == -1)
	{
		printf("send signal fail\n");
		return -1;
	}
	printf("send signal success\n");
	sleep(2);
}