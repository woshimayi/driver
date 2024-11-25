/*
 * @*************************************:
 * @FilePath: /driver/user/C/time-clock.c
 * @version:
 * @Author: dof
 * @Date: 2021-07-13 11:00:32
 * @LastEditors: dof
 * @LastEditTime: 2021-07-27 15:47:22
 * @Descripttion:  获取系统 进程 线程 uptime real spent time
 * @**************************************:
 */
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


#if 1
#include <stdio.h>
#include <sys/time.h>

int main() {
    struct timeval start, end;

    // 获取操作前的当前时间
    gettimeofday(&start, NULL);

    // 执行一些操作（例如，一个简单的循环）
    for (long i = 0; i < 1000000000; ++i);

    // 获取操作后的当前时间
    gettimeofday(&end, NULL);

    // 计算时间差
    double duration = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1e6;

    // 打印时间差
    printf("花费的时间: %.6f 秒\n", duration);

    return 0;
}
#else
int main(int argc, char *argv[])
{
	struct timespec time1 = {0, 0};

	clock_gettime(CLOCK_REALTIME, &time1);  //系统实时时间 获取相对于1970到现在的秒数
	printf("CLOCK_REALTIME: %lu, %lu\n", time1.tv_sec, time1.tv_nsec);

	clock_gettime(CLOCK_MONOTONIC,
	              &time1); //  从系统启动这一刻起开始计时,不受系统时间被用户改变的影响
	printf("CLOCK_MONOTONIC: %lu, %lu\n", time1.tv_sec, time1.tv_nsec);

	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1); // 本进程到当前代码系统CPU花费的时间
	printf("CLOCK_PROCESS_CPUTIME_ID: %lu, %lu\n", time1.tv_sec, time1.tv_nsec);

	clock_gettime(CLOCK_THREAD_CPUTIME_ID, &time1); // 本线程到当前代码系统CPU花费的时间
	printf("CLOCK_THREAD_CPUTIME_ID: %lu, %lu\n", time1.tv_sec, time1.tv_nsec);
	printf("\n%lu\n", time(NULL));

	return 0;
}
#endif