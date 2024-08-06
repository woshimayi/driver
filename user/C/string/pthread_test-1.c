/*
 * @*************************************:
 * @FilePath: /user/C/pthread_test-1.c
 * @version:
 * @Author: dof
 * @Date: 2022-08-26 17:47:58
 * @LastEditors: dof
 * @LastEditTime: 2022-08-26 17:53:18
 * @Descripttion: 阻塞模式
 * @**************************************:
 */

//使用互斥量解决多线程抢占资源的问题
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>

char *buf[5]; //字符指针数组  全局变量
int pos; //用于指定上面数组的下标

// 1.定义互斥量
pthread_mutex_t mutex;

void *task(void *p)
{ 
	// 3.使用互斥量进行加锁
	pthread_mutex_lock(&mutex);

	buf[pos] = (char *)p;
	printf("p = %s\n", (char *)p);
	sleep(1);
	pos++;
	
	// 4.使用互斥量进行解锁
	pthread_mutex_unlock(&mutex);
}

int main(void)
{
	// 2.初始化互斥量, 默认属性
	pthread_mutex_init(&mutex, NULL);
	
	// 1.启动一个线程 向数组中存储内容
	pthread_t tid, tid2;
	pthread_create(&tid, NULL, task, (void *)"zhangfei");
	pthread_create(&tid2, NULL, task, (void *)"guanyu"); // 2.主线程进程等待,并且打印最终的结果
	pthread_join(tid, NULL);
	pthread_join(tid2, NULL);
	
	// 5.销毁互斥量
	pthread_mutex_destroy(&mutex);

	int i = 0;
	printf("字符指针数组中的内容是：");
	for (i = 0; i < pos; ++i)
	{
		printf("%s ", buf[i]);
	}
	printf("\n");
	return 0;
}