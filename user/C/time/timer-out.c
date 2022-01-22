/*
 * @*************************************: 
 * @FilePath: /user/C/time/timer-out.c
 * @version: 
 * @Author: dof
 * @Date: 2021-10-20 19:33:39
 * @LastEditors: dof
 * @LastEditTime: 2021-12-13 19:04:55
 * @Descripttion: 定时器
 * @**************************************: 
 */

/*
	setitimer() 还可以根据进程的不同运行状态来控制定时功能的运行状态。
	setitimer() 是一个可以在本轮定时任务完成后自动重启下一轮定时的定时机制，
	当然是否自动重启取决于启动定时器时参数2的值。
	在上面函数参数2的释义中，struct itimerval 中的 it_value 成员表示定时器当前的值，
	它可以设置一个秒值及微秒值，其实就是你想让这个定时器在多少时间以后启动，
	立即启动则将 it_value 的值全设成0。
	而 it_interval 则表示你想定时多少时间。
	假如我们给 it_value 设置了一个值，但 it_interval 却全设成0，就表示定时器将在 it_value 
	设置的时间以后发出一个定时到时信号，此后就不会再自动重启下一轮定时任务了
*/



#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <sys/time.h> //包含setitimer()函数
#include <stdlib.h>
#include <signal.h>

//包含signal()函数
static int count = 0;
// struct itimerval 中的 it_value 成员表示定时器当前的值，它可以设置一个秒值及微秒值，其实就是你想让这个定时器在多少时间以后启动，立即启动则将 it_value 的值全设成0
static struct itimerval oldtv;

void set_timer()
{
	struct itimerval itv;
	itv.it_interval.tv_sec = 5;
	itv.it_interval.tv_usec = 0;
	itv.it_value.tv_sec = 1;
	itv.it_value.tv_usec = 0;
	setitimer(ITIMER_REAL, &itv, &oldtv);  //此函数为linux的api,不是c的标准库函数
}

void signal_handler(int m)
{
	printf("%d\n", count);
	if (m = SIGALRM)
	{
		count ++;
	}
}

void func()
{
	printf("%s %d\n", __FUNCTION__, __LINE__);
	signal(SIGALRM, signal_handler);  //注册当接收到SIGALRM时会发生是么函数；
	set_timer();  //启动定时器
}

int main()
{
	func();
	return 1;
}
