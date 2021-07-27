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
//指向函数的指针作函数参数,这里第一个参数是为指向函数的指针服务的，
{
    //不能写成void Caller2(int (*ptr)(int n))，这样的定义语法错误。
    (*ptr)(n);
    return;
}
