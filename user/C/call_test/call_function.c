#include <stdlib.h>
#include <stdio.h>
#include "call_function.h"

int Test2(void *num)
{
    printf("num = %f\n", *(double *)num);
    return 0;
}

int Test3(void *num)
{
    printf("char = %s\n", (char *)num);
    return 0;
}

int Test4(void *num)
{
    vos_error("int = %d\n", *(int *)num);
    return 0;
}

void Caller2(void *n, int (*ptr)())
//ָ������ָ������������,�����һ��������Ϊָ������ָ�����ģ�
{
    //����д��void Caller2(int (*ptr)(int n))�������Ķ����﷨����
    (*ptr)(n);
    return;
}
