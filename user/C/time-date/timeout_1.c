#include <stdio.h>
#include <windows.h>
//���̺߳���

char flag = 0;
char result = 0;

DWORD WINAPI longTimeFunction(LPVOID pM)
{
    while (1)
    {
        printf("opreating...\n");
        Sleep(20);
    }
    result = 1;
    flag = 1;
}

//����������ν��������ʵ�������߳�ִ�еĺ�����
int main()
{
    flag = 0;
    HANDLE longTimeFunctionHandle = CreateThread(NULL, 0, longTimeFunction, NULL, 0, NULL);
    WaitForSingleObject(longTimeFunctionHandle, 200);
    if (flag == 0)
    {
        printf("Timeout!\n");
        result = -1;
    }
    printf("result: %d\n", result);

    return 0;
}
