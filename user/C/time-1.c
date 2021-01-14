/*
	Name:
	Copyright:
	Author:
	Date: 15/09/20 17:37
	Description: ��ʱ�� ��ʱ ѭ��
*/



#include <stdio.h>
#include <string.h>

#include <sys/time.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>

// ���׼���������Ϣ�������û�ʱ�䵽��
void prompt_info(int signo)
{
    printf("time is running out\n");
}

// �����źŴ�������
void *init_sigaction(void *args)
{
    struct sigaction tact;
    /*�źŵ���Ҫִ�е�����������Ϊprompt_info*/
    tact.sa_handler = prompt_info;
    tact.sa_flags = 0;
    /*��ʼ���źż�*/
    sigemptyset(&tact.sa_mask);
    /*�����źŴ�������*/
    sigaction(SIGALRM, &tact, NULL);

    struct itimerval value;
    /*�趨ִ�������ʱ����Ϊ2��0΢��*/
    value.it_value.tv_sec = 15;
    value.it_value.tv_usec = 0;
    /*�趨��ʼʱ�����ҲΪ2��0΢��*/
    value.it_interval = value.it_value;
    /*���ü�ʱ��ITIMER_REAL*/
    setitimer(ITIMER_REAL, &value, NULL);
}

int main()
{
    init_sigaction(NULL);

    while (1);

    return 0;

}