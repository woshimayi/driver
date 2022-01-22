/*
 * @*************************************: 
 * @FilePath: /user/C/time/timer-out.c
 * @version: 
 * @Author: dof
 * @Date: 2021-10-20 19:33:39
 * @LastEditors: dof
 * @LastEditTime: 2021-12-13 19:04:55
 * @Descripttion: ��ʱ��
 * @**************************************: 
 */

/*
	setitimer() �����Ը��ݽ��̵Ĳ�ͬ����״̬�����ƶ�ʱ���ܵ�����״̬��
	setitimer() ��һ�������ڱ��ֶ�ʱ������ɺ��Զ�������һ�ֶ�ʱ�Ķ�ʱ���ƣ�
	��Ȼ�Ƿ��Զ�����ȡ����������ʱ��ʱ����2��ֵ��
	�����溯������2�������У�struct itimerval �е� it_value ��Ա��ʾ��ʱ����ǰ��ֵ��
	����������һ����ֵ��΢��ֵ����ʵ���������������ʱ���ڶ���ʱ���Ժ�������
	���������� it_value ��ֵȫ���0��
	�� it_interval ���ʾ���붨ʱ����ʱ�䡣
	�������Ǹ� it_value ������һ��ֵ���� it_interval ȴȫ���0���ͱ�ʾ��ʱ������ it_value 
	���õ�ʱ���Ժ󷢳�һ����ʱ��ʱ�źţ��˺�Ͳ������Զ�������һ�ֶ�ʱ������
*/



#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <sys/time.h> //����setitimer()����
#include <stdlib.h>
#include <signal.h>

//����signal()����
static int count = 0;
// struct itimerval �е� it_value ��Ա��ʾ��ʱ����ǰ��ֵ������������һ����ֵ��΢��ֵ����ʵ���������������ʱ���ڶ���ʱ���Ժ����������������� it_value ��ֵȫ���0
static struct itimerval oldtv;

void set_timer()
{
	struct itimerval itv;
	itv.it_interval.tv_sec = 5;
	itv.it_interval.tv_usec = 0;
	itv.it_value.tv_sec = 1;
	itv.it_value.tv_usec = 0;
	setitimer(ITIMER_REAL, &itv, &oldtv);  //�˺���Ϊlinux��api,����c�ı�׼�⺯��
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
	signal(SIGALRM, signal_handler);  //ע�ᵱ���յ�SIGALRMʱ�ᷢ����ô������
	set_timer();  //������ʱ��
}

int main()
{
	func();
	return 1;
}
