/*
 * @*************************************: 
 * @FilePath: /user/C/string/pthread_one.c
 * @version: 
 * @Author: dof
 * @Date: 2021-10-20 19:33:39
 * @LastEditors: dof
 * @LastEditTime: 2023-08-10 15:22:01
 * @Descripttion:  主进程等待子线程执行完
 * @**************************************: 
 */
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

void *thread_function(void *arg)
{
	int i;
	for (i = 0; i < 20; i++)
	{
		printf("Thread says hi! \n");
		sleep(1);
	}
	return NULL;
}
int main(void)
{
	pthread_t mythread;

	if (pthread_create(&mythread, NULL, thread_function, NULL))
	{
		printf("error creating thread.");
		abort();
	}
	if (pthread_join(mythread, NULL))
	{
		printf("error joining thread.");
		abort();
	}
	exit(0);
}

