#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc,char *argv[])
{
	struct timespec time1 = {0, 0};
	
	clock_gettime(CLOCK_REALTIME, &time1);  //ϵͳʵʱʱ�� 
	printf("CLOCK_REALTIME: %lu, %lu\n", time1.tv_sec, time1.tv_nsec);
	
	clock_gettime(CLOCK_MONOTONIC, &time1); //  ��ϵͳ������һ����ʼ��ʱ,����ϵͳʱ�䱻�û��ı��Ӱ��
	printf("CLOCK_MONOTONIC: %lu, %lu\n", time1.tv_sec, time1.tv_nsec);
	
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1); // �����̵���ǰ����ϵͳCPU���ѵ�ʱ��
	printf("CLOCK_PROCESS_CPUTIME_ID: %lu, %lu\n", time1.tv_sec, time1.tv_nsec);
	
	clock_gettime(CLOCK_THREAD_CPUTIME_ID, &time1); // ���̵߳���ǰ����ϵͳCPU���ѵ�ʱ��
	printf("CLOCK_THREAD_CPUTIME_ID: %lu, %lu\n", time1.tv_sec, time1.tv_nsec);
	printf("\n%lu\n", time(NULL));
	
	return 0;
}
