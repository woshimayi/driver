/*
 * @*************************************:
 * @FilePath: /driver/user/C/tiemout.c
 * @version:
 * @Author: dof
 * @Date: 2021-07-13 11:00:32
 * @LastEditors: dof
 * @LastEditTime: 2021-07-27 15:19:01
 * @Descripttion:  windows call time function
 * @**************************************:
 */
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <setjmp.h>

jmp_buf j;

/**
 * ʱ���жϺ���
 */
void PASCAL OneMilliSecondProc(UINT wTimerID, UINT msg, DWORD dwUser, DWORD dwl, DWORD dw2)
{
    printf("Timout!\n");
    longjmp(j, 1);
}

int longTimeFunction()
{
    while (1)
    {
        printf("operating...\n");
        Sleep(1000);
    }

    return 0;
}

int main()
{
    HANDLE hHandle;

    UINT wTimerRes_1ms; //����ʱ����
    UINT wAccuracy;     //����ֱ���
    UINT TimerID_1ms;   //���嶨ʱ�����
    wTimerRes_1ms = 5000;
    if ((TimerID_1ms = timeSetEvent(
                           wTimerRes_1ms,
                           wAccuracy,
                           (LPTIMECALLBACK)OneMilliSecondProc, // �ص�����
                           (DWORD)(1),                         // �û����͵��ص����������ݣ�
                           TIME_PERIODIC                       //���ڵ��ö�ʱ������
                       )) == 0)
    {
        printf("start!!!!!!!!!!!\n");
    }
    else
    {
        printf("end!!!!!!!!!!!\n");
    }

    int temp = 0;
    if (setjmp(j) == 0)
    {
        temp = longTimeFunction();
    }
    else
    {
        printf("xxxxxx...\n");
        temp = -1;
    }

    printf("%d\n", temp);

    return 0;
}
