/*************************************************************************
    > File Name       : posix_timer.c
    > Author          :
    > Mail            :
    > Created Time    :
 ************************************************************************/
#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>

void  function_timer()
{
	time_t t;
	char p[32];
	time(&t);
	//strftime(p, sizeof(p), "%T", localtime(&t));
	strftime(p, sizeof(p), "%Y-%m-%d %H:%M:%S", localtime(&t));
	printf("debug -----> date: %s \n", p);
}

void timer(int timeout)
{
	int ret;
	timer_t timer;
	struct sigevent evp;
	struct timespec spec;
	struct itimerspec time_value;

	evp.sigev_value.sival_ptr = &timer;
	/*��ʱ������ʱ�������һ���ź�*/
	evp.sigev_notify = SIGEV_SIGNAL;
	evp.sigev_signo = SIGUSR1;
	signal(SIGUSR1, function_timer);

	/*ʱ��ԴѡCLOCK_MONOTONIC��Ҫ�ǿ��ǵ�ϵͳ��ʵʱʱ�ӿ��ܻ���
	�������й����и��ģ����Դ���һ���Ĳ�ȷ���ԣ���CLOCK_MONOTONIC
	�򲻻ᣬ��Ϊ�ȶ�*/
	ret = timer_create(CLOCK_MONOTONIC, &evp, &timer);
	if (ret)
		perror("timer_create");

	time_value.it_interval.tv_sec = timeout;      /*ÿ�봥��һ��*/
	time_value.it_interval.tv_nsec = 0;
	clock_gettime(CLOCK_MONOTONIC, &spec);
	time_value.it_value.tv_sec = spec.tv_sec + 1;      /*5�������*/
	time_value.it_value.tv_nsec = spec.tv_nsec + 0;

	ret = timer_settime(timer, CLOCK_MONOTONIC, &time_value, NULL);
	if (ret)
		perror("timer_settime");
}
int main(int argc, char *argv[])
{
	timer(10);
	while (1)
	{
		sleep(1);
		printf("aaaaaaa \n");
	}

}
