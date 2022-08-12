/*
 * @*************************************: 
 * @FilePath: /user/C/string/for_performance.c
 * @version: 
 * @Author: dof
 * @Date: 2022-07-05 15:47:13
 * @LastEditors: dof
 * @LastEditTime: 2022-07-05 19:24:02
 * @Descripttion: 优化循环时间性能
 * @**************************************: 
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>


#if 0

#define LOOP_NUM	1000000
int main(int argc, char *argv[])
{
	long data[LOOP_NUM];
	long rand_num = 500000;
	struct timeval tv1, tv2;
	long i = 0; 

	for (i = 0; i < LOOP_NUM; ++i)
	{
		data[i] = i;
	}

	gettimeofday(&tv1, 0);
	for (i = 0; i < LOOP_NUM; ++i)
	{
		if (data[i] == rand_num)
		{
			printf("hit rand_num. i = %ld \n", i);
			break;
		}
	}
	gettimeofday(&tv2, 0);

	long us1 = tv1.tv_sec * 1000000 + tv1.tv_usec;
	long us2 = tv2.tv_sec * 1000000 + tv2.tv_usec;

	printf("time elapse: %ld \n", us2 - us1);
	return 0;
}
#else

#define LOOP_NUM	1000000
int main(int argc, char *argv[])
{
	long data[LOOP_NUM + 1];	// add another room
	long rand_num = 500000;
	struct timeval tv1, tv2;
	long i = 0;

	for ( i = 0; i < LOOP_NUM; ++i)
	{
		data[i] = i;
	}

	data[LOOP_NUM] = rand_num;  // add a sentinel
	printf("%d\n", data[LOOP_NUM]);

	gettimeofday(&tv1, 0);
	i = 0;
	while (1)
	{
		if (data[i] == rand_num)
		{
			if (i != LOOP_NUM)
			{
				printf("hit rand_num. i = %ld \n", i);
				break;
			}
		}
		++i;
	}
	gettimeofday(&tv2, 0);

	long us1 = tv1.tv_sec * 1000000 + tv1.tv_usec;
	long us2 = tv2.tv_sec * 1000000 + tv2.tv_usec;

	printf("time elapse: %ld \n", us2 - us1);
	return 0;
}

#endif
